#include <windows.h>
#include <vector>

#include <Zydis/Zydis.h>

#define ZYDIS_STATIC_BUILD

#include "Binary/Binary.hpp"

struct DecoderInstruction
{
	ZydisDecodedInstruction insn;
	ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
};

class Decoder
{
private:
	Binary* bin;
	ZydisFormatter formatter;
	ZydisDecoder decoder;
	DecoderInstruction intr;
public:
	static Decoder* load(Binary* bin)
	{
		auto* decoder = new Decoder();
		decoder->bin = bin;
		ZydisDecoderInit(&decoder->decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
		ZydisFormatterInit(&decoder->formatter, ZYDIS_FORMATTER_STYLE_INTEL);
		return decoder;
	}

	DecoderInstruction* Decode(uint64_t rva)
	{
		const void* bytes = bin->ToRaw(rva);
		if (!bytes)
			return nullptr;
		ZyanStatus status = ZydisDecoderDecodeFull(
			&decoder,
			bytes,
			15,
			&intr.insn,
			intr.operands);
		return &intr;
	}

	void log(DecoderInstruction* intr, uint64_t rip)
	{
		char text[256];
		ZydisFormatterFormatInstruction(
			&formatter,
			&intr->insn,
			intr->operands,
			intr->insn.operand_count_visible,
			text,
			sizeof(text),
			rip,          // runtime VA for absolute addresses; or ZYDIS_RUNTIME_ADDRESS_NONE
			ZYAN_NULL);
		printf("%s\n", text);  // e.g. "mov rax, rcx"
		return;
	}

	void unload()
	{
		return;
	}
};

enum IR_MNMONIC
{
	IR_SUB,
	IR_ADD,
	IR_MULT,
	IR_DIV,
	IR_MOV
};

enum IR_OPERAND_TYPE
{
	IR_OPERAND_IMM,
	IR_OPERAND_REG,
	IR_OPERAND_MEM
};

struct IR_OPERAND
{
	IR_OPERAND_TYPE type;
	union
	{
		struct
		{
			uint64_t value;
		}imm;
		struct
		{
			int reg_idx;
		}reg;
		struct
		{
			int reg_idx;
		}mem;
	};
};

struct IR
{
	IR_MNMONIC mnmonic;
	IR_OPERAND op[2];
	int op_count;
};

class Lifter
{
	
public:
	static bool run(DecoderInstruction* intr, IR* ir)
	{
		switch (intr->insn.mnemonic)
		{
		case ZYDIS_MNEMONIC_SUB:
		{
			ir->mnmonic = IR_SUB;
			return false;
		}break;
		}
		return false;
	}
};


int main()
{
	auto bin = Binary::Load(L"C:\\Users\\Admin\\Desktop\\EfiLoader.exe");

	auto startup = bin->GetStartAddress(0);

	auto decode = Decoder::load(bin);
	
	auto intr = decode->Decode(startup);
	
	IR ir;
	if(!Lifter::run(intr, &ir))
		decode->log(intr, startup);



	decode->unload();

	bin->Unload();
	return 0;
}
