#define ZYDIS_STATIC_BUILD
#include "ZydisWrapper.hpp"

#include "Binary/PeImage.hpp"

#include <climits>
#include <cstring>

static bool OperandWritesMemory(const ZydisDecodedOperand& operand)
{
	return (operand.actions & ZYDIS_OPERAND_ACTION_MASK_WRITE) != 0;
}

static bool MemoryUsesRsp(const ZydisDecodedOperand& operand, int64_t* disp_out)
{
	if (operand.type != ZYDIS_OPERAND_TYPE_MEMORY)
		return false;

	const auto& mem = operand.mem;
	if (mem.base != ZYDIS_REGISTER_RSP && mem.base != ZYDIS_REGISTER_ESP)
		return false;

	if (mem.index != ZYDIS_REGISTER_NONE)
		return false;

	if (disp_out)
		*disp_out = mem.disp.has_displacement ? mem.disp.value : 0;

	return true;
}

static int64_t MemoryRspSlotIndex(const ZydisDecodedOperand& operand)
{
	int64_t disp = 0;
	if (!MemoryUsesRsp(operand, &disp))
		return INT64_MIN;

	if (disp % 8 != 0)
		return INT64_MIN;

	return disp / 8;
}

DecodedInsn::DecodedInsn(
	uint64_t rip, const ZydisDecodedInstruction& insn, const ZydisDecodedOperand* operands)
	: rip_(rip), insn_(insn)
{
	memcpy(operands_, operands, sizeof(operands_[0]) * insn_.operand_count_visible);
}

bool DecodedInsn::CalcBranchTarget(uint64_t* out_target) const
{
	if (!out_target)
		return false;

	for (ZyanU8 i = 0; i < insn_.operand_count_visible; ++i)
	{
		const auto& operand = operands_[i];
		if (operand.type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
			continue;

		if (!operand.imm.is_relative)
			continue;

		uint64_t target = 0;
		if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(&insn_, &operand, rip_, &target)))
			continue;

		*out_target = target;
		return true;
	}

	return false;
}

bool DecodedInsn::TryGetMemoryOperandAbsolute(uint64_t* out_address) const
{
	if (!out_address)
		return false;

	for (ZyanU8 i = 0; i < insn_.operand_count_visible; ++i)
	{
		const auto& operand = operands_[i];
		if (operand.type != ZYDIS_OPERAND_TYPE_MEMORY)
			continue;

		uint64_t address = 0;
		if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(&insn_, &operand, rip_, &address)))
			continue;

		*out_address = address;
		return true;
	}

	return false;
}

bool DecodedInsn::IsIndirectControlFlow() const
{
	if (insn_.meta.category != ZYDIS_CATEGORY_UNCOND_BR &&
		insn_.meta.category != ZYDIS_CATEGORY_COND_BR &&
		insn_.meta.category != ZYDIS_CATEGORY_CALL)
	{
		return false;
	}

	uint64_t ignored = 0;
	return !CalcBranchTarget(&ignored);
}

bool DecodedInsn::UpdatesCallStackContext(int* ret_slot_index, bool* ret_slot_tainted) const
{
	if (!ret_slot_index || !ret_slot_tainted)
		return false;

	switch (insn_.mnemonic)
	{
	case ZYDIS_MNEMONIC_PUSH:
	case ZYDIS_MNEMONIC_PUSHF:
	case ZYDIS_MNEMONIC_PUSHFD:
	case ZYDIS_MNEMONIC_PUSHFQ:
		(*ret_slot_index)++;
		break;

	case ZYDIS_MNEMONIC_POP:
	case ZYDIS_MNEMONIC_POPF:
	case ZYDIS_MNEMONIC_POPFD:
	case ZYDIS_MNEMONIC_POPFQ:
		if (*ret_slot_index == 0)
			*ret_slot_tainted = true;
		(*ret_slot_index)--;
		if (*ret_slot_index < 0)
			*ret_slot_index = 0;
		break;

	default:
		break;
	}

	if (insn_.mnemonic == ZYDIS_MNEMONIC_SUB || insn_.mnemonic == ZYDIS_MNEMONIC_ADD)
	{
		bool rsp_dest = false;
		uint64_t imm = 0;

		for (ZyanU8 i = 0; i < insn_.operand_count_visible; ++i)
		{
			const auto& operand = operands_[i];
			if (operand.type == ZYDIS_OPERAND_TYPE_REGISTER &&
				(operand.actions & ZYDIS_OPERAND_ACTION_MASK_WRITE) &&
				(operand.reg.value == ZYDIS_REGISTER_RSP || operand.reg.value == ZYDIS_REGISTER_ESP))
			{
				rsp_dest = true;
			}

			if (operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
				imm = operand.imm.value.u;
		}

		if (rsp_dest && imm != 0)
		{
			const int slots = static_cast<int>(imm / 8);
			if (insn_.mnemonic == ZYDIS_MNEMONIC_SUB)
				(*ret_slot_index) += slots;
			else
			{
				if (*ret_slot_index < slots)
					*ret_slot_tainted = true;
				(*ret_slot_index) -= slots;
				if (*ret_slot_index < 0)
					*ret_slot_index = 0;
			}
		}
	}

	for (ZyanU8 i = 0; i < insn_.operand_count_visible; ++i)
	{
		const auto& operand = operands_[i];
		if (!OperandWritesMemory(operand))
			continue;

		const int64_t slot = MemoryRspSlotIndex(operand);
		if (slot == INT64_MIN)
			continue;

		if (slot == *ret_slot_index)
			*ret_slot_tainted = true;
	}

	return insn_.meta.category == ZYDIS_CATEGORY_RET;
}

void DecodedInsn::TextIntel(char* buffer, size_t capacity) const
{
	if (!buffer || capacity == 0)
		return;

	static ZydisFormatter formatter{};
	static bool formatter_init = false;
	if (!formatter_init)
	{
		ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);
		formatter_init = true;
	}

	ZydisFormatterFormatInstruction(
		&formatter, &insn_, operands_, insn_.operand_count_visible, buffer, capacity, rip_, ZYAN_NULL);
}

ZydisWrapper::ZydisWrapper()
{
	ZydisDecoderInit(&decoder_, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
	ZydisFormatterInit(&formatter_, ZYDIS_FORMATTER_STYLE_INTEL);
}

DecodedInsn* ZydisWrapper::DecodeRip(uint64_t rip, const PeImage& image)
{
	uint8_t buffer[ZYDIS_MAX_INSTRUCTION_LENGTH]{};
	if (!image.ReadVa(rip, buffer, sizeof(buffer)))
		return nullptr;

	ZydisDecodedInstruction insn{};
	ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT]{};

	if (!ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder_, buffer, sizeof(buffer), &insn, operands)))
		return nullptr;

	return new DecodedInsn(rip, insn, operands);
}
