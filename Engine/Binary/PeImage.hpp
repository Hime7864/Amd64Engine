#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class PeImage
{
public:
	struct SectionInfo
	{
		uint32_t virtual_address = 0;
		uint32_t virtual_size = 0;
		uint32_t raw_offset = 0;
		uint32_t raw_size = 0;
		uint32_t characteristics = 0;
	};

	bool Load(const wchar_t* path);

	uint64_t image_base() const { return image_base_; }
	uint32_t size_of_image() const { return size_of_image_; }
	uint32_t entry_point_rva() const { return entry_point_rva_; }

	bool ContainsVa(uint64_t va, uint32_t access_bytes) const;
	bool ReadVa(uint64_t va, void* dst, size_t n) const;

	const std::vector<uint8_t>& image() const { return image_; }
	const std::vector<SectionInfo>& sections() const { return sections_; }

private:
	bool VaToOffset(uint64_t va, size_t& out_offset) const;

	uint64_t image_base_ = 0;
	uint32_t size_of_image_ = 0;
	uint32_t entry_point_rva_ = 0;
	std::vector<uint8_t> image_;
	std::vector<SectionInfo> sections_;
};
