#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "PeImage.hpp"

#include <Windows.h>

#include <algorithm>
#include <fstream>

static bool ReadFileToBuffer(const wchar_t* path, std::vector<uint8_t>& out)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
		return false;

	const auto size = file.tellg();
	if (size <= 0)
		return false;

	out.resize(static_cast<size_t>(size));
	file.seekg(0, std::ios::beg);
	return static_cast<bool>(file.read(reinterpret_cast<char*>(out.data()), size));
}

static const PeImage::SectionInfo* FindSectionForRva(
	const std::vector<PeImage::SectionInfo>& sections, uint32_t rva)
{
	for (const auto& section : sections)
	{
		const uint32_t section_end = section.virtual_address +
			std::max(section.virtual_size, section.raw_size);
		if (rva >= section.virtual_address && rva < section_end)
			return &section;
	}
	return nullptr;
}

bool PeImage::Load(const wchar_t* path)
{
	std::vector<uint8_t> file_bytes;
	if (!ReadFileToBuffer(path, file_bytes))
		return false;

	if (file_bytes.size() < sizeof(IMAGE_DOS_HEADER))
		return false;

	const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(file_bytes.data());
	if (dos->e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	const size_t nt_offset = static_cast<size_t>(dos->e_lfanew);
	if (nt_offset + sizeof(IMAGE_NT_HEADERS64) > file_bytes.size())
		return false;

	const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(file_bytes.data() + nt_offset);
	if (nt->Signature != IMAGE_NT_SIGNATURE)
		return false;

	if (nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64)
		return false;

	const auto& optional = nt->OptionalHeader;
	if (optional.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		return false;

	image_base_ = optional.ImageBase;
	size_of_image_ = optional.SizeOfImage;
	entry_point_rva_ = optional.AddressOfEntryPoint;

	image_.assign(size_of_image_, 0);
	sections_.clear();

	const uint32_t headers_size = optional.SizeOfHeaders;
	const size_t copy_headers = std::min<size_t>(headers_size, file_bytes.size());
	if (copy_headers > image_.size())
		return false;

	memcpy(image_.data(), file_bytes.data(), copy_headers);

	const auto* section_header = IMAGE_FIRST_SECTION(nt);
	for (uint16_t i = 0; i < nt->FileHeader.NumberOfSections; ++i)
	{
		const auto& sh = section_header[i];

		SectionInfo info{};
		info.virtual_address = sh.VirtualAddress;
		info.virtual_size = sh.Misc.VirtualSize;
		info.raw_offset = sh.PointerToRawData;
		info.raw_size = sh.SizeOfRawData;
		info.characteristics = sh.Characteristics;
		sections_.push_back(info);

		if (sh.PointerToRawData == 0 || sh.SizeOfRawData == 0)
			continue;

		const size_t dst_offset = sh.VirtualAddress;
		const size_t copy_size = std::min<size_t>(sh.SizeOfRawData, size_of_image_ - dst_offset);
		if (dst_offset + copy_size > image_.size())
			continue;

		const size_t src_offset = sh.PointerToRawData;
		if (src_offset + copy_size > file_bytes.size())
			continue;

		memcpy(image_.data() + dst_offset, file_bytes.data() + src_offset, copy_size);
	}

	return true;
}

bool PeImage::VaToOffset(uint64_t va, size_t& out_offset) const
{
	if (va < image_base_)
		return false;

	const uint64_t rva = va - image_base_;
	if (rva >= size_of_image_)
		return false;

	// image_ is a flat buffer mapped at RVAs (see Load).
	out_offset = static_cast<size_t>(rva);
	return true;
}

bool PeImage::ContainsVa(uint64_t va, uint32_t access_bytes) const
{
	if (access_bytes == 0)
		return false;

	if (va < image_base_)
		return false;

	const uint64_t rva = va - image_base_;
	if (rva >= size_of_image_)
		return false;

	if (rva + access_bytes > size_of_image_)
		return false;

	if (rva + access_bytes < rva)
		return false;

	return true;
}

bool PeImage::ReadVa(uint64_t va, void* dst, size_t n) const
{
	if (!dst || n == 0)
		return false;

	if (!ContainsVa(va, static_cast<uint32_t>(n)))
		return false;

	size_t offset = 0;
	if (!VaToOffset(va, offset))
		return false;

	if (offset + n > image_.size())
		return false;

	memcpy(dst, image_.data() + offset, n);
	return true;
}
