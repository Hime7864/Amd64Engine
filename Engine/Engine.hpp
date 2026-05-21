#pragma once
#include "structures.hpp"

class AssemblyState
{
private:
	UINT64 GPR[16];
	XMMWORD XMM[16];
	UINT64 Advancement;
	BYTE* RIP;
	EFLAGS FLAGS;
	UINT64 GsBase;
	UINT64 FsBase;
	MNEMONICPREFIX Prefix;

	void log_Prefix();
	void log_ModRM(MODRM* modrm);

	UINT64 GetDisplacementPtr();

	// basic arithmetic

	bool service_add();
	bool service_sub();
	bool service_inc();
	bool service_dec();
	bool service_mul();
	bool service_div();

	bool service_adc();
	bool service_sbb();

	bool service_or();
	bool service_xor();
	bool service_and();

	bool service_test();
	bool service_cmp();

	bool service_push();
	bool service_pop();

	bool service_mov();
	bool service_lea();

	bool service_jmp();
	bool service_call();
	bool service_syscall();
	bool service_ret();

	bool service_rol();
	bool service_ror();
	bool service_rcl();
	bool service_rcr();
	bool service_sal();
	bool service_shr();
	bool service_shl();
	bool service_sar();

	//special operations

	bool service_stosd();
	bool service_bt();
	// decoder

	bool decode_mnemonic();
public:
	void SetRip(PVOID rip);
	UINT64 GetRip();

	void SetGPR(int index, UINT64 value);
	UINT64 GetGPR(int index);

	void SetGsBase(UINT64 value) { GsBase = value; }
	void SetFsBase(UINT64 value) { FsBase = value; }

	bool step();
};