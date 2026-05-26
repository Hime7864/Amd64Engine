#pragma once
#include "structures.hpp"

class AssemblyState
{
private:
	UINT64 GPR[16];
	XMMWORD XMM[16];
	BYTE* RIP;
	EFLAGS FLAGS;
	UINT64 GsBase;
	UINT64 FsBase;
	MNEMONICPREFIX Prefix;

	void log_Prefix();
	void log_ModRM(MODRM* modrm);

	UINT64 GetDisplacementPtr();
	
	bool get_msb(UINT64 value, UINT8 bits);
	bool get_lsb(UINT64 value, UINT8 bits);

	bool read_bit(UINT64 BitBase, UINT64 BitOffset, UINT8 bits);
	void write_bit(UINT64 BitBase, UINT64 BitOffset, UINT8 bits, bool State);

	bool decode_mnemonic();
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

	bool service_setcc();
	bool service_bt();

	//special operations

	bool service_stosd();
	
public:
	UINT64 GetRip();
	void SetRip(PVOID rip);
	
	UINT64 GetGPR(int index);
	void SetGPR(int index, UINT64 value);
	
	void GetFlags(EFLAGS* flags);
	void SetFlags(EFLAGS* flags);

	void SetGsBase(UINT64 value) { GsBase = value; }
	void SetFsBase(UINT64 value) { FsBase = value; }

	bool step();
	void log_step();
};