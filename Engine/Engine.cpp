#include "Engine.hpp"
#include <cstdio>
#include <cstring>

#include "Services/add.hpp"
#include "Services/and.hpp"
#include "Services/bt.hpp"
#include "Services/call.hpp"
#include "Services/cmp.hpp"
#include "Services/decode.hpp"
#include "Services/dec.hpp"
#include "Services/inc.hpp"
#include "Services/jmp.hpp"
#include "Services/lea.hpp"
#include "Services/mov.hpp"
#include "Services/or.hpp"
#include "Services/pop.hpp"
#include "Services/push.hpp"
#include "Services/rcl.hpp"
#include "Services/rcr.hpp"
#include "Services/ret.hpp"
#include "Services/rol.hpp"
#include "Services/ror.hpp"
#include "Services/sal.hpp"
#include "Services/sar.hpp"
#include "Services/shl.hpp"
#include "Services/shr.hpp"
#include "Services/stosd.hpp"
#include "Services/sub.hpp"
#include "Services/syscall.hpp"
#include "Services/test.hpp"
#include "Services/xor.hpp"

UINT64 AssemblyState::GetDisplacementPtr()
{
	auto modrm = (MODRM*)(&RIP[1]);
	switch (modrm->Mode)
	{
	case EMODE::MEM_0_BIT_DISP:
	{
		if (modrm->RegisterMemory == (int)EGPR::RSP)//SIB
		{
			auto modrm2 = (MODRM*)(&RIP[2]);
			auto mutiplier = 1ull << (BYTE)modrm2->Mode;
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm2_register_memory = Prefix.B ? modrm2->RegisterMemory + 8 : modrm2->RegisterMemory;
			auto modrm2_register = Prefix.X ? modrm2->Register + 8 : modrm2->Register;

			if (modrm2->RegisterMemory == (BYTE)EGPR::RBP)
			{


				if (modrm2->Register == (int)EGPR::RSP)
				{
					auto imm = *(INT32*)(&RIP[3]);
					auto ptr = (UINT64)imm;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					//printf("[%x]\n", imm);

					RIP += 7;
					return ptr;
				}
				else
				{
					auto imm = *(INT32*)(&RIP[3]);
					auto ptr = GPR[modrm2_register] * mutiplier + imm;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					//printf("[%x + r%i * %i]\n", imm, modrm2_register, mutiplier);

					RIP += 7;
					return ptr;
				}
			}
			else if (modrm2->RegisterMemory == (BYTE)EGPR::RSP)
			{
				if (modrm2->RegisterMemory == modrm2->Register)
				{
					auto ptr = GPR[modrm2->Register];

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					//printf("[rsp]\n");

					RIP += 3;
					return ptr;
				}
				else
				{
					auto ptr = GPR[modrm2->Register] + GPR[modrm2_register] * mutiplier;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					//printf("[rsp + r%i * %i]\n", modrm2->Register, mutiplier);

					RIP += 3;
					return ptr;
				}

			}
			else
			{
				auto ptr = GPR[modrm2_register_memory] + GPR[modrm2_register] * mutiplier;

				if (Prefix.GS)
					ptr += GsBase;
				else if (Prefix.FS)
					ptr += FsBase;

				//printf("[r%i + r%i * %i]\n", modrm2_register_memory, modrm2_register, mutiplier);
				RIP += 3;
				return ptr;
			}
		}
		else if (modrm->RegisterMemory == (BYTE)EGPR::RBP)
		{
			auto imm = *(INT32*)(&RIP[2]);
			RIP += 6;
			auto ptr = (INT64)RIP + imm;

			if (Prefix.GS)
				ptr += GsBase;
			else if (Prefix.FS)
				ptr += FsBase;

			//printf("[rip + %x]\n", imm);

			return ptr;
		}
		else//REG
		{

			auto ptr = GPR[modrm->RegisterMemory];

			if (Prefix.GS)
				ptr += GsBase;
			else if (Prefix.FS)
				ptr += FsBase;

			//printf("[r%i]\n", modrm->RegisterMemory);

			RIP += 2;
			return ptr;
		}
	}break;
	case EMODE::MEM_8_BIT_DISP:
	{
		if (modrm->RegisterMemory == (BYTE)EGPR::RSP)
		{
			auto modrm2 = (MODRM*)(&RIP[2]);
			auto mutiplier = 1ull << (BYTE)modrm2->Mode;
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm2_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
			auto modrm2_register = Prefix.X ? modrm2->Register + 8 : modrm2->Register;
			auto imm = *(INT8*)(&RIP[3]);

			if (modrm2->RegisterMemory == (BYTE)EGPR::RSP)
			{
				if (modrm2->RegisterMemory == modrm2->Register)
				{
					auto ptr = GPR[modrm2->Register] + (INT64)imm;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					//printf("!!!! [rsp + %x]\n", imm);

					RIP += 4;
					return ptr;
				}
				else
				{
					auto ptr = GPR[modrm2->RegisterMemory] + GPR[modrm2_register] * mutiplier + (INT64)imm;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					//printf("!!!! [rsp + r%i * %i + %x]\n", modrm2->Register, mutiplier, imm);

					RIP += 4;
					return ptr;
				}
			}
			else
			{
				auto ptr = GPR[modrm_register] + GPR[modrm2_register] * mutiplier + (INT64)imm;

				if (Prefix.GS)
					ptr += GsBase;
				else if (Prefix.FS)
					ptr += FsBase;

				//printf("!!!! [r%i + r%i * %i+ %x]\n", modrm->Register, modrm2->Register, mutiplier, imm);

				RIP += 4;
				return ptr;
			}
		}
		else
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
			auto imm = *(INT8*)(&RIP[2]);
			auto ptr = (INT64)GPR[modrm_register_memory] + (INT64)imm;

			if (Prefix.GS)
				ptr += GsBase;
			else if (Prefix.FS)
				ptr += FsBase;

			RIP += 3;
			return ptr;
		}

	}break;
	case EMODE::MEM_32_BIT_DISP:
	{
		if (modrm->RegisterMemory == (BYTE)EGPR::RSP)
		{
			auto modrm2 = (MODRM*)(&RIP[2]);
			auto mutiplier = 1ull << (BYTE)modrm2->Mode;
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm2_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
			auto modrm2_register = Prefix.X ? modrm2->Register + 8 : modrm2->Register;
			auto imm = *(INT32*)(&RIP[3]);

			if (modrm2->RegisterMemory == (BYTE)EGPR::RSP)
			{
				if (modrm2->RegisterMemory == modrm2->Register)
				{
					auto ptr = GPR[modrm2->Register] + (INT64)imm;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					//printf("!!!! [rsp + %x]\n", imm);

					RIP += 7;
					return ptr;
				}
				else
				{
					auto ptr = GPR[modrm2->RegisterMemory] + GPR[modrm2_register] * mutiplier + (INT64)imm;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					//printf("!!!! [rsp + r%i * %i + %x]\n", modrm2->Register, mutiplier, imm);

					RIP += 7;
					return 0;
				}
			}
			else
			{
				auto ptr = GPR[modrm_register] + GPR[modrm2_register] * mutiplier + (INT64)imm;

				if (Prefix.GS)
					ptr += GsBase;
				else if (Prefix.FS)
					ptr += FsBase;

				//printf("!!!! [r%i + r%i * %i+ %x]\n", modrm->Register, modrm2->Register, mutiplier, imm);

				RIP += 7;
				return ptr;
			}
		}
		else
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
			auto imm = *(INT32*)(&RIP[2]);
			auto ptr = (INT64)GPR[modrm_register_memory] + (INT64)imm;

			if (Prefix.GS)
				ptr += GsBase;
			else if (Prefix.FS)
				ptr += FsBase;

			RIP += 6;
			return ptr;
		}
	}break;
	case EMODE::REG_TO_REG:
	{
		auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
		auto ptr = &GPR[modrm_register_memory];
		RIP += 2;
		return (UINT64)ptr;
	}break;
	};
	return 0;
}

void AssemblyState::log_ModRM(MODRM* modrm)
{
	printf(" -> ModRM: [ mode: %d, regmem: %d, reg: %d ]\n", modrm->Mode, modrm->RegisterMemory, modrm->Register);
}

void AssemblyState::log_Prefix()
{
	printf(" -> Prefix: [ ");
	if (Prefix.W)
		printf("W, ");
	if (Prefix.R)
		printf("R, ");
	if (Prefix.X)
		printf("X, ");
	if (Prefix.B)
		printf("B, ");
	if (Prefix.FS)
		printf("FS, ");
	if (Prefix.GS)
		printf("GS, ");
	if (Prefix.OperandSize)
		printf("OperandSizeOverride, ");
	if (Prefix.AddressSize)
		printf("AddressSizeOverride, ");
	printf("]\n");
}

UINT64 AssemblyState::GetRip()
{
	return (UINT64)RIP;
}

void AssemblyState::SetRip(PVOID rip)
{
	RIP = (BYTE*)rip;
	return;
}

UINT64 AssemblyState::GetGPR(int index)
{
	return GPR[index % 16];
}

void AssemblyState::SetGPR(int index, UINT64 value)
{
	GPR[index % 16] = value;
	return;
}

void AssemblyState::GetFlags(EFLAGS* flags)
{
	flags = &FLAGS;
	return;
}

void AssemblyState::SetFlags(EFLAGS* flags)
{
	FLAGS = *flags;
	return;
}

bool AssemblyState::step()
{
	bool status = false;

	status = decode_mnemonic();

	if(RIP == 0x0)
		return false;

	return status;
}

void AssemblyState::log_step()
{
	printf("[%p] CF %i, PF %i, AF %i, ZF %i, SF %i, TF %i, IF %i, DF %i, OF %i\n",
		RIP, FLAGS.CF, FLAGS.PF, FLAGS.AF, FLAGS.ZF, FLAGS.SF, FLAGS.TF, FLAGS.IF, FLAGS.DF, FLAGS.OF);
	printf("RAX %p ", GPR[(int)EGPR::RAX]);
	printf("RCX %p ", GPR[(int)EGPR::RCX]);
	printf("RDX %p ", GPR[(int)EGPR::RDX]);
	printf("RBX %p \n", GPR[(int)EGPR::RBX]);
	printf("RSP %p ", GPR[(int)EGPR::RSP]);
	printf("RBP %p ", GPR[(int)EGPR::RBP]);
	printf("RSI %p ", GPR[(int)EGPR::RSI]);
	printf("RDI %p \n", GPR[(int)EGPR::RDI]);
	printf("R8  %p ", GPR[(int)EGPR::R8]);
	printf("R9  %p ", GPR[(int)EGPR::R9]);
	printf("R10 %p ", GPR[(int)EGPR::R10]);
	printf("R11 %p \n", GPR[(int)EGPR::R11]);
	printf("R12 %p ", GPR[(int)EGPR::R12]);
	printf("R13 %p ", GPR[(int)EGPR::R13]);
	printf("R14 %p ", GPR[(int)EGPR::R14]);
	printf("R15 %p \n", GPR[(int)EGPR::R15]);

	for (int i = 0; i < 16; i++)
	{
		auto xmm = XMM[i];
		printf("XMM%02i: %08x %08x %08x %08x", i, xmm.u32.data[0], xmm.u32.data[1], xmm.u32.data[2], xmm.u32.data[3]);
		if(i % 2)
			printf("\n");
		else
			printf(" ");
	}
	printf("\n");
	return;
}