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
		printf(" [error] -> %s\n", text);  // e.g. "mov rax, rcx"
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
	IR_SUB_F, // x86 ALU sub — writes flags listed in flags_written
	IR_ADD_F, // x86 ALU add — writes flags listed in flags_written
	IR_CMP,   // flag-only sub (no dest write)
	IR_TEST,  // flag-only and (no dest write)
	IR_MULT,
	IR_DIV,
	IR_MOV,
	IR_JMP
};

const char* IrMnmonicString(IR_MNMONIC mnmonic)
{
	switch (mnmonic)
	{
	case IR_SUB:
		return "SUB";
	case IR_ADD:
		return "ADD";
	case IR_SUB_F:
		return "SUB_F";
	case IR_ADD_F:
		return "ADD_F";
	case IR_CMP:
		return "CMP";
	case IR_TEST:
		return "TEST";
	case IR_MULT:
		return "MULT";
	case IR_DIV:
		return "DIV";
	case IR_MOV:
		return "MOV";
	case IR_JMP:
		return "JMP";
	default:
		return "UNK";
	};
}

// Explicit flag defs for later opts (e.g. dead SUB_F -> SUB if no reader before clobber).
enum IR_FLAG : uint32_t
{
	IR_FLAG_CF = 1u << 0,
	IR_FLAG_PF = 1u << 1,
	IR_FLAG_AF = 1u << 2,
	IR_FLAG_ZF = 1u << 3,
	IR_FLAG_SF = 1u << 4,
	IR_FLAG_OF = 1u << 5,
};

// GPR 0-15, status flags 16-21, address temps from 32+.
enum IR_REG : int
{
	IR_RAX = 0,
	IR_RCX = 1,
	IR_RDX = 2,
	IR_RBX = 3,
	IR_RSP = 4,
	IR_RBP = 5,
	IR_RSI = 6,
	IR_RDI = 7,
	IR_R8 = 8,
	IR_R9 = 9,
	IR_R10 = 10,
	IR_R11 = 11,
	IR_R12 = 12,
	IR_R13 = 13,
	IR_R14 = 14,
	IR_R15 = 15,

	IR_CF = 16,
	IR_PF = 17,
	IR_AF = 18,
	IR_ZF = 19,
	IR_SF = 20,
	IR_OF = 21,

	IR_VREG_BASE = 32,

	IR_NONE = -1,
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
		} imm;
		struct
		{
			int reg_idx; // IR_RAX..IR_R15, flag regs, or vreg >= IR_VREG_BASE
		} reg;
		struct
		{
			// After lifting, memory is always a simple deref of one reg:
			//   [rN]  where N may be a vreg holding the computed EA
			int reg_idx;
		} mem;
	};
};

struct IR
{
	IR_MNMONIC mnmonic;
	IR_OPERAND op[2];
	int op_count;
	// Bitmask of IR_FLAG_* this op defines. Plain SUB/ADD (EA/call) leave this 0.
	uint32_t flags_written;

	void log() const
	{
		printf("%s", IrMnmonicString(mnmonic));
		for (int i = 0; i < op_count; ++i)
		{
			printf(i == 0 ? " " : ", ");
			const IR_OPERAND& o = op[i];
			switch (o.type)
			{
			case IR_OPERAND_IMM:
				printf("0x%llx", static_cast<unsigned long long>(o.imm.value));
				break;
			case IR_OPERAND_REG:
				printf("r%d", o.reg.reg_idx);
				break;
			case IR_OPERAND_MEM:
				printf("[r%d]", o.mem.reg_idx);
				break;
			}
		}
		if (flags_written)
		{
			printf("  ; flags:");
			if (flags_written & IR_FLAG_CF) printf(" CF");
			if (flags_written & IR_FLAG_PF) printf(" PF");
			if (flags_written & IR_FLAG_AF) printf(" AF");
			if (flags_written & IR_FLAG_ZF) printf(" ZF");
			if (flags_written & IR_FLAG_SF) printf(" SF");
			if (flags_written & IR_FLAG_OF) printf(" OF");
		}
		printf("\n");
	}
};

class Lifter
{
	static int next_vreg_;

	static int AllocVreg()
	{
		return next_vreg_++;
	}

	static void ResetVregs()
	{
		next_vreg_ = IR_VREG_BASE;
	}

	static IR_OPERAND MakeReg(int reg_idx)
	{
		IR_OPERAND o{};
		o.type = IR_OPERAND_REG;
		o.reg.reg_idx = reg_idx;
		return o;
	}

	static IR_OPERAND MakeImm(uint64_t value)
	{
		IR_OPERAND o{};
		o.type = IR_OPERAND_IMM;
		o.imm.value = value;
		return o;
	}

	static IR_OPERAND MakeMem(int addr_reg)
	{
		IR_OPERAND o{};
		o.type = IR_OPERAND_MEM;
		o.mem.reg_idx = addr_reg;
		return o;
	}

	static IR MakeBinary(IR_MNMONIC m, const IR_OPERAND& a, const IR_OPERAND& b)
	{
		IR ir{};
		ir.mnmonic = m;
		ir.op[0] = a;
		ir.op[1] = b;
		ir.op_count = 2;
		ir.flags_written = 0;
		return ir;
	}

	static IR MakeUnary(IR_MNMONIC m, const IR_OPERAND& a)
	{
		IR ir{};
		ir.mnmonic = m;
		ir.op[0] = a;
		ir.op_count = 1;
		ir.flags_written = 0;
		return ir;
	}

	// Flags this x86 insn defines (modified / forced / undefined all kill prior defs).
	static uint32_t MapWrittenFlags(const ZydisDecodedInstruction* insn)
	{
		if (!insn || !insn->cpu_flags)
			return 0;

		const ZydisAccessedFlagsMask w =
			insn->cpu_flags->modified |
			insn->cpu_flags->set_0 |
			insn->cpu_flags->set_1 |
			insn->cpu_flags->undefined;

		uint32_t out = 0;
		if (w & ZYDIS_CPUFLAG_CF) out |= IR_FLAG_CF;
		if (w & ZYDIS_CPUFLAG_PF) out |= IR_FLAG_PF;
		if (w & ZYDIS_CPUFLAG_AF) out |= IR_FLAG_AF;
		if (w & ZYDIS_CPUFLAG_ZF) out |= IR_FLAG_ZF;
		if (w & ZYDIS_CPUFLAG_SF) out |= IR_FLAG_SF;
		if (w & ZYDIS_CPUFLAG_OF) out |= IR_FLAG_OF;
		return out;
	}

	static int MapGpr(ZydisRegister reg)
	{
		if (reg == ZYDIS_REGISTER_NONE)
			return IR_NONE;

		const ZydisRegister full = ZydisRegisterGetLargestEnclosing(
			ZYDIS_MACHINE_MODE_LONG_64, reg);

		if (full >= ZYDIS_REGISTER_RAX && full <= ZYDIS_REGISTER_R15)
			return static_cast<int>(full - ZYDIS_REGISTER_RAX);

		return IR_NONE;
	}

	// Expand ModRM/SIB into vregs (>15), return address reg for [rN].
	static bool ExpandMem(
		const ZydisDecodedInstruction* insn,
		const ZydisDecodedOperand* src,
		uint64_t rip,
		std::vector<IR>* out,
		int* out_addr_reg)
	{
		const int base = MapGpr(src->mem.base);
		const int index = MapGpr(src->mem.index);
		const uint8_t scale = src->mem.scale ? src->mem.scale : 1;
		const bool has_disp = src->mem.disp.has_displacement != 0;
		const int64_t disp = has_disp ? src->mem.disp.value : 0;
		const bool rip_relative = (src->mem.base == ZYDIS_REGISTER_RIP);

		// RIP-relative or absolute disp-only: fold to a concrete address imm.
		if (rip_relative || (base == IR_NONE && index == IR_NONE))
		{
			uint64_t abs = 0;
			if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(insn, src, rip, &abs)))
				return false;

			const int v = AllocVreg();
			out->push_back(MakeBinary(IR_MOV, MakeReg(v), MakeImm(abs)));
			*out_addr_reg = v;
			return true;
		}

		const int v = AllocVreg();

		if (index != IR_NONE)
		{
			// mov v, index
			out->push_back(MakeBinary(IR_MOV, MakeReg(v), MakeReg(index)));

			// mult v, imm:scale  (when scale != 1)
			if (scale > 1)
				out->push_back(MakeBinary(IR_MULT, MakeReg(v), MakeImm(scale)));

			// add v, base
			if (base != IR_NONE)
				out->push_back(MakeBinary(IR_ADD, MakeReg(v), MakeReg(base)));
		}
		else if (base != IR_NONE)
		{
			// mov v, base
			out->push_back(MakeBinary(IR_MOV, MakeReg(v), MakeReg(base)));
		}
		else
		{
			return false;
		}

		// add v, imm:disp
		if (has_disp && disp != 0)
			out->push_back(MakeBinary(IR_ADD, MakeReg(v), MakeImm(static_cast<uint64_t>(disp))));

		*out_addr_reg = v;
		return true;
	}

	static bool LiftOperand(
		const ZydisDecodedInstruction* insn,
		const ZydisDecodedOperand* src,
		uint64_t rip,
		std::vector<IR>* prelude,
		IR_OPERAND* dst)
	{
		switch (src->type)
		{
		case ZYDIS_OPERAND_TYPE_REGISTER:
		{
			const int idx = MapGpr(src->reg.value);
			if (idx < 0 || idx > 15)
				return false;
			*dst = MakeReg(idx);
			return true;
		}

		case ZYDIS_OPERAND_TYPE_IMMEDIATE:
			*dst = MakeImm(src->imm.is_signed
				? static_cast<uint64_t>(src->imm.value.s)
				: src->imm.value.u);
			return true;

		case ZYDIS_OPERAND_TYPE_MEMORY:
		{
			int addr_reg = IR_NONE;
			if (!ExpandMem(insn, src, rip, prelude, &addr_reg))
				return false;
			*dst = MakeMem(addr_reg);
			return true;
		}

		default:
			return false;
		}
	}

	// Resolve near branch target: rel imm -> abs, reg, or [mem] (load pointer).
	static bool LiftBranchTarget(
		const ZydisDecodedInstruction* insn,
		const ZydisDecodedOperand* target_op,
		uint64_t rip,
		std::vector<IR>* prelude,
		IR_OPERAND* target)
	{
		if (target_op->type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
		{
			uint64_t abs = 0;
			if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(insn, target_op, rip, &abs)))
				return false;
			*target = MakeImm(abs);
			return true;
		}

		if (target_op->type == ZYDIS_OPERAND_TYPE_REGISTER)
			return LiftOperand(insn, target_op, rip, prelude, target);

		if (target_op->type == ZYDIS_OPERAND_TYPE_MEMORY)
		{
			int ea_reg = IR_NONE;
			if (!ExpandMem(insn, target_op, rip, prelude, &ea_reg))
				return false;

			const int fn_reg = AllocVreg();
			prelude->push_back(MakeBinary(IR_MOV, MakeReg(fn_reg), MakeMem(ea_reg)));
			*target = MakeReg(fn_reg);
			return true;
		}

		return false;
	}

public:
	static bool run(DecoderInstruction* intr, uint64_t rip, std::vector<IR>* out, uint64_t* pathA, uint64_t* pathB)
	{
		if (!intr || !out || !pathA || !pathB)
			return false;

		// Default: fall through to the next instruction.
		*pathA = rip + intr->insn.length;
		*pathB = 0;

		out->clear();
		ResetVregs();

		switch (intr->insn.mnemonic)
		{
		case ZYDIS_MNEMONIC_SUB:
		case ZYDIS_MNEMONIC_ADD:
		case ZYDIS_MNEMONIC_MOV:
		case ZYDIS_MNEMONIC_CMP:
		case ZYDIS_MNEMONIC_TEST:
		{
			if (intr->insn.operand_count_visible != 2)
				return false;

			std::vector<IR> prelude;
			IR_OPERAND a{};
			IR_OPERAND b{};

			if (!LiftOperand(&intr->insn, &intr->operands[0], rip, &prelude, &a))
				return false;
			if (!LiftOperand(&intr->insn, &intr->operands[1], rip, &prelude, &b))
				return false;

			out->insert(out->end(), prelude.begin(), prelude.end());

			IR_MNMONIC m = IR_MOV;
			bool writes_flags = false;
			switch (intr->insn.mnemonic)
			{
			case ZYDIS_MNEMONIC_ADD:
				m = IR_ADD_F;
				writes_flags = true;
				break;
			case ZYDIS_MNEMONIC_SUB:
				m = IR_SUB_F;
				writes_flags = true;
				break;
			case ZYDIS_MNEMONIC_CMP:
				m = IR_CMP;
				writes_flags = true;
				break;
			case ZYDIS_MNEMONIC_TEST:
				m = IR_TEST;
				writes_flags = true;
				break;
			default:
				break;
			}

			IR ir = MakeBinary(m, a, b);
			if (writes_flags)
				ir.flags_written = MapWrittenFlags(&intr->insn);
			out->push_back(ir);
			break;
		}

		case ZYDIS_MNEMONIC_PUSH:
		{
			// push r/m/imm  ->  sub rsp, N ; mov [rsp], value
			// Evaluate the value before adjusting RSP (push [rsp+...] safety).
			if (intr->insn.operand_count_visible < 1)
				return false;

			const ZydisDecodedOperand* src = &intr->operands[0];
			const uint64_t push_bytes = src->size ? (src->size / 8) : 8;

			std::vector<IR> prelude;
			IR_OPERAND value{};

			if (src->type == ZYDIS_OPERAND_TYPE_REGISTER ||
				src->type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
			{
				if (!LiftOperand(&intr->insn, src, rip, &prelude, &value))
					return false;
			}
			else if (src->type == ZYDIS_OPERAND_TYPE_MEMORY)
			{
				int ea_reg = IR_NONE;
				if (!ExpandMem(&intr->insn, src, rip, &prelude, &ea_reg))
					return false;

				const int tmp = AllocVreg();
				prelude.push_back(MakeBinary(IR_MOV, MakeReg(tmp), MakeMem(ea_reg)));
				value = MakeReg(tmp);
			}
			else
			{
				return false;
			}

			out->insert(out->end(), prelude.begin(), prelude.end());
			out->push_back(MakeBinary(IR_SUB, MakeReg(IR_RSP), MakeImm(push_bytes)));
			out->push_back(MakeBinary(IR_MOV, MakeMem(IR_RSP), value));
			break;
		}

		case ZYDIS_MNEMONIC_JMP:
		{
			// Near jmp only: jmp rel, jmp r64, jmp [mem] (+ displacement expand).
			if (intr->insn.operand_count_visible < 1)
				return false;

			std::vector<IR> prelude;
			IR_OPERAND target{};
			if (!LiftBranchTarget(&intr->insn, &intr->operands[0], rip, &prelude, &target))
				return false;

			out->insert(out->end(), prelude.begin(), prelude.end());
			out->push_back(MakeUnary(IR_JMP, target));

			// Imm jmp: pathA = absolute target. Reg/mem jmp: unknown for now.
			if (target.type == IR_OPERAND_IMM)
				*pathA = target.imm.value;
			else
				*pathA = 0;
			*pathB = 0;
			break;
		}

		case ZYDIS_MNEMONIC_CALL:
		{
			// Near call only. Forms: call rel32, call r64, call [mem]
			if (intr->insn.operand_count_visible < 1)
				return false;

			const uint64_t return_rip = rip + intr->insn.length;

			std::vector<IR> prelude;
			IR_OPERAND target{};
			if (!LiftBranchTarget(&intr->insn, &intr->operands[0], rip, &prelude, &target))
				return false;

			// Compute target before touching RSP (important for call [rsp+...]).
			out->insert(out->end(), prelude.begin(), prelude.end());

			// call == push return address + jmp target
			out->push_back(MakeBinary(IR_SUB, MakeReg(IR_RSP), MakeImm(8)));
			out->push_back(MakeBinary(IR_MOV, MakeMem(IR_RSP), MakeImm(return_rip)));
			out->push_back(MakeUnary(IR_JMP, target));

			// Trace into the callee when target is known; return site on pathB.
			if (target.type == IR_OPERAND_IMM)
				*pathA = target.imm.value;
			else
				*pathA = 0;
			*pathB = return_rip;
			break;
		}

		default:
			return false;
		}

		for (const IR& ir : *out)
			ir.log();

		return true;
	}
};

int Lifter::next_vreg_ = IR_VREG_BASE;

struct LiftedIntr
{
	uint64_t intr_rip;
	DecoderInstruction intr;
	std::vector<IR> ir;
};

struct BasicBlock
{
	std::vector<LiftedIntr> steps;
};

class CFG
{
private:

public:

};

int main()
{
	auto bin = Binary::Load(L"C:\\Users\\Admin\\Desktop\\EfiLoader.exe");

	auto startup = bin->GetStartAddress(0);

	auto decode = Decoder::load(bin);

	BasicBlock current;
	for (int i = 0; i < 1000; i++)
	{
		uint64_t pathA = 0, pathB = 0;
		std::vector<IR> ir;
		auto intr = decode->Decode(startup);
		if (!Lifter::run(intr, startup, &ir, &pathA, &pathB))
		{
			decode->log(intr, startup);
			break;
		}
		startup = pathA;
	}

	decode->unload();

	bin->Unload();
	return 0;
}
