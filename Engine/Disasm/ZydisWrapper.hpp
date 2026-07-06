#pragma once

#define ZYDIS_STATIC_BUILD
#include <Zydis/Zydis.h>

#include <cstdint>

class PeImage;

class DecodedInsn
{
public:
	DecodedInsn(uint64_t rip, const ZydisDecodedInstruction& insn, const ZydisDecodedOperand* operands);

	uint64_t Rip() const { return rip_; }
	uint32_t Length() const { return insn_.length; }
	ZydisInstructionCategory Category() const { return insn_.meta.category; }
	ZydisMnemonic Mnemonic() const { return insn_.mnemonic; }

	bool CalcBranchTarget(uint64_t* out_target) const;
	bool TryGetMemoryOperandAbsolute(uint64_t* out_address) const;
	bool IsIndirectControlFlow() const;
	bool UpdatesCallStackContext(int* ret_slot_index, bool* ret_slot_tainted) const;

	void TextIntel(char* buffer, size_t capacity) const;

private:
	uint64_t rip_;
	ZydisDecodedInstruction insn_;
	ZydisDecodedOperand operands_[ZYDIS_MAX_OPERAND_COUNT];
};

class ZydisWrapper
{
public:
	ZydisWrapper();

	DecodedInsn* DecodeRip(uint64_t rip, const PeImage& image);

private:
	ZydisDecoder decoder_;
	ZydisFormatter formatter_;
};
