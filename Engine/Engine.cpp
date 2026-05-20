#include "Engine.hpp"
#include <cstdio>
#include <cstring>

#include "Services/add.hpp"
#include "Services/and.hpp"
#include "Services/btr.hpp"
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

					printf("[%x + r%i * %i]\n", imm, modrm2_register, mutiplier);

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

			//printf("!!!! [r%i + %x]\n", modrm_register_memory, imm);

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

void AssemblyState::SetRip(PVOID rip)
{
	RIP = (BYTE*)rip;
	return;
}

UINT64 AssemblyState::GetRip()
{
	return (UINT64)RIP;
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

bool AssemblyState::step()
{
	bool status = false;

	status = decode_mnemonic();

	if(RIP == 0x0)
		return false;

	return status;
}
