#pragma once

#include "PeImage.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class DecodedInsn;
class ZydisWrapper;

enum EntrypointType
{
	ENTRYPOINT_TYPE_START,
	ENTRYPOINT_TYPE_TLS,
	ENTRYPOINT_TYPE_EXPORT
};

struct BinaryEntrypoint
{
	EntrypointType type = ENTRYPOINT_TYPE_START;
	uint64_t address = 0;
};

struct ImportSymbol
{
	std::string module;
	std::string name;
};

class Binary
{
public:
	~Binary();

	static Binary* Load(const wchar_t* file_path);

	void* GetStartup() const;
	const std::vector<BinaryEntrypoint>& Entrypoints() const { return entrypoints_; }
	const PeImage& Image() const { return image_; }

	DecodedInsn* DecodeRip(uint64_t rip);

	bool TryResolveImport(uint64_t iat_va, const ImportSymbol** out_symbol) const;
	void FormatImportLabel(uint64_t iat_va, char* buffer, size_t capacity) const;
	size_t ImportCount() const { return imports_by_iat_va_.size(); }

private:
	void BuildEntrypoints();
	void BuildImports();

	PeImage image_;
	ZydisWrapper* disasm_ = nullptr;
	std::vector<BinaryEntrypoint> entrypoints_;
	std::unordered_map<uint64_t, ImportSymbol> imports_by_iat_va_;
};
