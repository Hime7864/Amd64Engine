#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Binary.hpp"

#include "Disasm/ZydisWrapper.hpp"

#include <Windows.h>

#include <cstdio>
#include <cstring>

Binary::~Binary()
{
	delete disasm_;
}

Binary* Binary::Load(const wchar_t* file_path)
{
	auto* binary = new Binary();
	if (!binary->image_.Load(file_path))
	{
		delete binary;
		return nullptr;
	}

	binary->disasm_ = new ZydisWrapper();
	binary->BuildEntrypoints();
	binary->BuildImports();
	return binary;
}

void* Binary::GetStartup() const
{
	for (const auto& entrypoint : entrypoints_)
	{
		if (entrypoint.type == ENTRYPOINT_TYPE_START)
			return reinterpret_cast<void*>(entrypoint.address);
	}
	return nullptr;
}

DecodedInsn* Binary::DecodeRip(uint64_t rip)
{
	if (!disasm_)
		return nullptr;

	return disasm_->DecodeRip(rip, image_);
}

void Binary::BuildEntrypoints()
{
	entrypoints_.clear();

	const uint64_t image_base = image_.image_base();
	const uint32_t entry_rva = image_.entry_point_rva();
	if (entry_rva != 0)
	{
		BinaryEntrypoint start{};
		start.type = ENTRYPOINT_TYPE_START;
		start.address = image_base + entry_rva;
		entrypoints_.push_back(start);
	}

	const uint64_t image_end = image_base + image_.size_of_image();
	const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(image_.image().data());
	if (dos->e_magic != IMAGE_DOS_SIGNATURE)
		return;

	const size_t nt_offset = static_cast<size_t>(dos->e_lfanew);
	if (nt_offset + sizeof(IMAGE_NT_HEADERS64) > image_.image().size())
		return;

	const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(image_.image().data() + nt_offset);
	const auto& optional = nt->OptionalHeader;

	const auto& tls_dir = optional.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
	if (tls_dir.VirtualAddress != 0 && tls_dir.Size != 0)
	{
		const uint64_t tls_va = image_base + tls_dir.VirtualAddress;
		IMAGE_TLS_DIRECTORY64 tls{};
		if (image_.ReadVa(tls_va, &tls, sizeof(tls)) && tls.AddressOfCallBacks != 0)
		{
			uint64_t callback_va = tls.AddressOfCallBacks;
			for (uint32_t index = 0; index < 64; ++index)
			{
				uint64_t callback = 0;
				if (!image_.ReadVa(callback_va, &callback, sizeof(callback)))
					break;

				if (callback == 0)
					break;

				if (callback < image_base || callback >= image_end)
					break;

				BinaryEntrypoint tls_entry{};
				tls_entry.type = ENTRYPOINT_TYPE_TLS;
				tls_entry.address = callback;
				entrypoints_.push_back(tls_entry);

				callback_va += sizeof(uint64_t);
			}
		}
	}

	const auto& export_dir = optional.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if (export_dir.VirtualAddress != 0 && export_dir.Size != 0)
	{
		const uint64_t export_va = image_base + export_dir.VirtualAddress;
		IMAGE_EXPORT_DIRECTORY exports{};
		if (!image_.ReadVa(export_va, &exports, sizeof(exports)))
			return;

		for (uint32_t i = 0; i < exports.NumberOfNames; ++i)
		{
			uint32_t name_rva = 0;
			const uint64_t name_ptr_va = image_base + exports.AddressOfNames + i * sizeof(uint32_t);
			if (!image_.ReadVa(name_ptr_va, &name_rva, sizeof(name_rva)))
				continue;

			uint16_t ordinal_index = 0;
			const uint64_t ordinal_ptr_va = image_base + exports.AddressOfNameOrdinals + i * sizeof(uint16_t);
			if (!image_.ReadVa(ordinal_ptr_va, &ordinal_index, sizeof(ordinal_index)))
				continue;

			uint32_t function_rva = 0;
			const uint64_t function_ptr_va = image_base + exports.AddressOfFunctions + ordinal_index * sizeof(uint32_t);
			if (!image_.ReadVa(function_ptr_va, &function_rva, sizeof(function_rva)))
				continue;

			if (function_rva == 0)
				continue;

			const uint64_t function_va = image_base + function_rva;
			if (function_va < image_base || function_va >= image_end)
				continue;

			BinaryEntrypoint export_entry{};
			export_entry.type = ENTRYPOINT_TYPE_EXPORT;
			export_entry.address = function_va;
			entrypoints_.push_back(export_entry);
		}
	}
}

bool Binary::TryResolveImport(uint64_t iat_va, const ImportSymbol** out_symbol) const
{
	const auto found = imports_by_iat_va_.find(iat_va);
	if (found == imports_by_iat_va_.end())
		return false;

	if (out_symbol)
		*out_symbol = &found->second;

	return true;
}

void Binary::FormatImportLabel(uint64_t iat_va, char* buffer, size_t capacity) const
{
	if (!buffer || capacity == 0)
		return;

	const ImportSymbol* symbol = nullptr;
	if (!TryResolveImport(iat_va, &symbol))
	{
		snprintf(buffer, capacity, "import@0x%llX", static_cast<unsigned long long>(iat_va));
		return;
	}

	snprintf(
		buffer,
		capacity,
		"%s!%s",
		symbol->module.c_str(),
		symbol->name.c_str());
}

void Binary::BuildImports()
{
	imports_by_iat_va_.clear();

	const uint64_t image_base = image_.image_base();
	const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(image_.image().data());
	if (dos->e_magic != IMAGE_DOS_SIGNATURE)
		return;

	const size_t nt_offset = static_cast<size_t>(dos->e_lfanew);
	if (nt_offset + sizeof(IMAGE_NT_HEADERS64) > image_.image().size())
		return;

	const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(image_.image().data() + nt_offset);
	const auto& optional = nt->OptionalHeader;
	const auto& import_dir = optional.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (import_dir.VirtualAddress == 0 || import_dir.Size == 0)
		return;

	const uint64_t import_desc_va = image_base + import_dir.VirtualAddress;
	for (uint32_t desc_index = 0; desc_index < 1024; ++desc_index)
	{
		IMAGE_IMPORT_DESCRIPTOR desc{};
		const uint64_t desc_va = import_desc_va + desc_index * sizeof(IMAGE_IMPORT_DESCRIPTOR);
		if (!image_.ReadVa(desc_va, &desc, sizeof(desc)))
			break;

		if (desc.Name == 0)
			break;

		char module_name[256]{};
		if (!image_.ReadVa(image_base + desc.Name, module_name, sizeof(module_name) - 1))
			continue;

		const uint32_t lookup_rva = desc.OriginalFirstThunk ? desc.OriginalFirstThunk : desc.FirstThunk;
		if (lookup_rva == 0 || desc.FirstThunk == 0)
			continue;

		for (uint32_t thunk_index = 0; thunk_index < 65536; ++thunk_index)
		{
			const uint64_t lookup_va = image_base + lookup_rva + thunk_index * sizeof(uint64_t);
			const uint64_t iat_va = image_base + desc.FirstThunk + thunk_index * sizeof(uint64_t);

			uint64_t thunk = 0;
			if (!image_.ReadVa(lookup_va, &thunk, sizeof(thunk)))
				break;

			if (thunk == 0)
				break;

			ImportSymbol symbol{};
			symbol.module = module_name;

			if (IMAGE_SNAP_BY_ORDINAL64(thunk))
			{
				char ordinal_name[32]{};
				snprintf(
					ordinal_name,
					sizeof(ordinal_name),
					"ordinal_%u",
					static_cast<unsigned>(IMAGE_ORDINAL64(thunk)));
				symbol.name = ordinal_name;
			}
			else
			{
				const uint64_t hint_name_va = image_base + thunk;
				char import_name[256]{};
				if (!image_.ReadVa(hint_name_va + 2, import_name, sizeof(import_name) - 1))
					continue;

				symbol.name = import_name;
			}

			imports_by_iat_va_[iat_va] = std::move(symbol);
		}
	}
}
