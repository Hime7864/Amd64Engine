#include <Windows.h>
#include <stdio.h>

struct EFLAGS
{
	BYTE CF : 1;
	BYTE reserved_1 : 1;
	BYTE PF : 1;
	BYTE reserved_2 : 1;
	BYTE AF : 1;
	BYTE reserved_3 : 1;
	BYTE ZF : 1;
	BYTE SF : 1;
	BYTE TF : 1;
	BYTE IF : 1;
	BYTE DF : 1;
	BYTE OF : 1;
};

enum struct EGPR : BYTE
{
	RAX = 0,
	RCX = 1,
	RDX = 2,
	RBX = 3,
	RSP = 4,
	RBP = 5,
	RSI = 6,
	RDI = 7,
	R8 = 8,
	R9 = 9,
	R10 = 10,
	R11 = 11,
	R12 = 12,
	R13 = 13,
	R14 = 14,
	R15 = 15
};

enum struct EMODE : BYTE
{
	MEM_0_BIT_DISP = 0,
	MEM_8_BIT_DISP = 1,
	MEM_32_BIT_DISP = 2,
	REG_TO_REG = 3
};

struct MODRM
{
	BYTE RegisterMemory : 3;
	BYTE Register : 3;
	EMODE Mode : 2;
};

struct MNEMONICPREFIX
{
	INT16 B : 1;
	INT16 X : 1;
	INT16 R : 1;
	INT16 W : 1;
	INT16 CS : 1;
	INT16 SS : 1;
	INT16 DS : 1;
	INT16 ES : 1;
	INT16 FS : 1;
	INT16 GS : 1;
	INT16 LOCK : 1;
	INT16 OperandSize : 1;
	INT16 AddressSize : 1;
	INT16 Repeated : 1;
};

class AssemblyState
{
private:
	UINT64 GPR[16];
	UINT64 Advancement;
	BYTE* RIP;
	EFLAGS FLAGS;
	UINT64 GsBase;
	UINT64 FsBase;
	MNEMONICPREFIX Prefix;

	void log_Prefix();
	void log_ModRM(MODRM* modrm);

	UINT64 GetDisplacementPtr();

	bool service_mov();
	bool service_lea();

	bool service_jmp();
	bool service_call();
	bool service_ret();

	bool service_push();

	bool service_sub();
	bool service_add();
	bool service_xor();

	bool service_test();
	bool service_cmp();

	bool service_stosd();

	bool decode_mnemonic();
public:
	void SetRip(PVOID rip);
	void SetGPR(int index, UINT64 value);
	bool step();
};

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
			auto modrm2_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
			auto modrm2_register = Prefix.X ? modrm2->Register + 8 : modrm2->Register;

			if (modrm2->RegisterMemory == (BYTE)EGPR::RBP)
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
				auto ptr = GPR[modrm_register] + GPR[modrm2_register] * mutiplier;

				if (Prefix.GS)
					ptr += GsBase;
				else if (Prefix.FS)
					ptr += FsBase;

				//printf("[r%i + r%i * %i]\n", modrm->Register, modrm2->Register, mutiplier);

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

void AssemblyState::SetGPR(int index, UINT64 value)
{
	GPR[index % 16] = value;
	return;
}

bool AssemblyState::service_mov()
{
	bool status = false;
	switch (*RIP)
	{
	case 0x0F:
	{
		RIP++;
		switch (*RIP)
		{
		case 0xB6:
		{
			auto modrm = (MODRM*)&RIP[1];
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)
				{
					GPR[modrm_register] = ((UINT64) * (BYTE*)ptr) & 0xFFull;
				}
				else
				{
					if (Prefix.OperandSize)
					{
						*(UINT16*)&GPR[modrm_register] = ((UINT64) * (BYTE*)ptr) & 0xFFull;
					}
					else
					{
						*(UINT32*)&GPR[modrm_register] = ((UINT64) * (BYTE*)ptr) & 0xFFull;
					}
				}
				status = true;
			}
			else
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
				if (Prefix.W)
				{
					GPR[modrm_register] = ((UINT64) * (BYTE*)&GPR[modrm_register_memory]) & 0xFFull;
				}
				else
				{
					if (Prefix.OperandSize)
					{
						*(UINT16*)&GPR[modrm_register] = ((UINT64) * (BYTE*)&GPR[modrm_register_memory]) & 0xFFull;
					}
					else
					{
						*(UINT32*)&GPR[modrm_register] = ((UINT64) * (BYTE*)&GPR[modrm_register_memory]) & 0xFFull;
					}
				}
				status = true;
			}
		}break;
		}
	}break;
	case 0x88:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
		auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

		*(BYTE*)&GPR[modrm_register_memory] = *(BYTE*)&GPR[modrm_register];

		RIP += 2;
		status = true;
	}break;
	case 0x89:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)
				{
					*(UINT64*)ptr = GPR[modrm_register];
				}
				else
				{
					*(UINT32*)ptr = *(UINT32*)&GPR[modrm_register];
				}
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				GPR[modrm_register_memory] = GPR[modrm_register];
			}
			else
			{
				*(UINT32*)&GPR[modrm_register_memory] = *(UINT32*)&GPR[modrm_register];
			}
			RIP += 2;
			status = true;
		}break;
		};

	}break;
	case 0x8B:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)
				{
					GPR[modrm_register] = *(UINT64*)ptr;
				}
				else
				{
					if (Prefix.OperandSize)
					{
						*(UINT16*)&GPR[modrm_register] = *(UINT16*)ptr;
					}
					else
					{
						*(UINT32*)&GPR[modrm_register] = *(UINT32*)ptr;
					}
				}
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
			if (Prefix.W)
			{
				GPR[modrm_register] = GPR[modrm_register_memory];
			}
			else
			{
				if (Prefix.OperandSize)
				{
					*(UINT16*)&GPR[modrm_register] = *(UINT16*)&GPR[modrm_register_memory];
				}
				else
				{
					*(UINT32*)&GPR[modrm_register] = *(UINT32*)&GPR[modrm_register_memory];
				}
			}
			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x8C:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
			GPR[modrm->RegisterMemory] = 0x2B;
			break;
		case 1:
			GPR[modrm->RegisterMemory] = 0x33;
			break;
		case 2:
			GPR[modrm->RegisterMemory] = 0x2B;
			break;
		case 3:
			GPR[modrm->RegisterMemory] = 0x2B;
			break;
		case 4:
			GPR[modrm->RegisterMemory] = 0x53;
			break;
		case 5:
			GPR[modrm->RegisterMemory] = 0x2B;
			break;
		}
		RIP += 2;
		status = true;
	}break;
	case 0xA4://MOVSB
	{
		auto src = GPR[(BYTE)EGPR::RSI];
		auto dst = GPR[(BYTE)EGPR::RDI];
		auto count = GPR[(BYTE)EGPR::RCX];
		memcpy((PVOID)dst, (PVOID)src, count);
		RIP++;
		status = true;
	}break;
	case 0xA5:
		break;//avx bullshit
	case 0xB0:
	case 0xB1:
	case 0xB2:
	case 0xB3:
	case 0xB4:
	case 0xB5:
	case 0xB6:
	case 0xB7:
	{
		auto reg = *RIP & 0x7;
		if(Prefix.B)
			reg += 8;
		*(BYTE*)&GPR[reg] = RIP[1];
		RIP += 2;
		status = true;
	}break;
	case 0xB8:
	case 0xB9:
	case 0xBA:
	case 0xBB:
	case 0xBC:
	case 0xBD:
	case 0xBE:
	case 0xBF:
	{
		auto reg = *RIP & 0x7;
		if (Prefix.B)
			reg += 8;

		if (Prefix.OperandSize)
		{
			*(WORD*)&GPR[reg] = *(WORD*)(&RIP[1]);
			RIP += 3;
			status = true;
		}
		else
		{
			if (Prefix.W)
			{
				GPR[reg] = *(UINT64*)(&RIP[1]);
				RIP += 9;
				status = true;
			}
			else
			{
				GPR[reg] = *(UINT32*)(&RIP[1]);
				RIP += 5;
				status = true;
			}
		}
	}break;
	case 0xC6:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto imm = *(UINT8*)(&RIP[2]);
		auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

		*(BYTE*)&GPR[modrm_register_memory] = imm;

		RIP += 3;
		status = true;
	}break;
	case 0xC7:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto imm = *(UINT32*)(&RIP[2]);
		auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

		if (Prefix.W)
		{
			GPR[modrm_register_memory] = imm;
		}
		else
		{
			if (Prefix.OperandSize)
			{
				*(UINT16*)&GPR[modrm_register_memory] = (UINT16)imm;
			}
			else
			{
				*(UINT32*)&GPR[modrm_register_memory] = imm;
			}
		}
		
		RIP += 6;
		status = true;

	}break;
	};

	if (status)
		printf("MOVE\n");

	return status;
}

bool AssemblyState::service_lea()
{
	bool status = false;
	auto modrm = (MODRM*)(&RIP[1]);
	switch (modrm->Mode)
	{
	case EMODE::MEM_0_BIT_DISP:
	case EMODE::MEM_8_BIT_DISP:
	case EMODE::MEM_32_BIT_DISP:
	{
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			if (Prefix.W)
			{
				GPR[modrm_register] = ptr;
			}
			else
			{
				if (Prefix.OperandSize)
				{
					*(WORD*)&GPR[modrm_register] = (WORD)ptr;
				}
				else
				{
					*(DWORD*)&GPR[modrm_register] = (DWORD)ptr;
				}
			}
			status = true;
		}
	}break;
	};

	if(status)
		printf("LEA\n");

	return status;
}

bool AssemblyState::service_jmp()
{
	bool status = false;
	auto opcode = (BYTE*)RIP;
	switch (*opcode)
	{
	case 0x74:
	{
		auto imm = *(INT8*)(opcode + 1);
		if (FLAGS.ZF == 0)
		{
			RIP += 2;
		}
		else
		{
			Advancement = (UINT64)RIP + 2;
			RIP += imm + 2;
		}
		
		status = true;
	}break;
	case 0xE9:
	{
		auto imm = *(INT32*)(opcode + 1);
		Advancement = (UINT64)RIP + 5;
		RIP += imm + 5;
		status = true;
	}break;
	case 0xEB:
	{
		auto imm = *(INT8*)(opcode + 1);
		Advancement = (UINT64)RIP + 2;
		RIP += imm + 2;
		status = true;
	}break;
	case 0xFF:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 4:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				Advancement = (UINT64)RIP;
				RIP = (BYTE*)*(UINT64*)ptr;
				status = true;
			}
		}break;
		case 5:
		{

		}break;
		}
	}break;
	};

	if (status)
		printf("JUMP\n");

	return status;
}

bool AssemblyState::service_call()
{
	auto status = false;


	switch (*RIP)
	{
	case 0xE8:
	{
		auto imm = *(INT32*)&RIP[1];
		GPR[(int)EGPR::RSP] -= 8;
		*(UINT64*)GPR[(int)EGPR::RSP] = (UINT64)(RIP + 5);
		Advancement = (UINT64)RIP + 5;
		RIP += imm + 5;
		status = true;
	}break;
	case 0xFF:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 2:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				GPR[(int)EGPR::RSP] -= 8;
				*(UINT64*)GPR[(int)EGPR::RSP] = (UINT64)RIP;
				Advancement = (UINT64)RIP;
				RIP = (BYTE*)*(UINT64*)ptr;
				status = true;
			}
		}break;
		case 3:
		{
		}break;
		}
	}break;
	};
	

	if (status)
		printf("CALL\n");

	return status;
}

bool AssemblyState::service_ret()
{
	auto status = false;
	Advancement = (UINT64)RIP + 1;
	RIP = (BYTE*)*(UINT64*)GPR[(int)EGPR::RSP];
	GPR[(int)EGPR::RSP] += 8;
	status = true;
	if (status)
		printf("RET\n");
	return status;
}

bool AssemblyState::service_push()
{
	bool status = false;
	switch (*RIP)
	{
	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
	{
		auto reg = *RIP & 0x7;
		if (Prefix.B)
			reg += 8;
		
		GPR[(int)EGPR::RSP] -= 8;
		*(UINT64*)GPR[(int)EGPR::RSP] = GPR[reg];
		RIP += 1;
		
		status = true;
	}break;
	case 0xFF:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 6:
		{
			switch (modrm->Mode)
			{
			case EMODE::MEM_0_BIT_DISP:
			case EMODE::MEM_8_BIT_DISP:
			case EMODE::MEM_32_BIT_DISP:
			{
				auto ptr = GetDisplacementPtr();
				if (ptr)
				{
					if (Prefix.OperandSize)
					{
						GPR[(int)EGPR::RSP] -= 2;
						*(UINT16*)GPR[(int)EGPR::RSP] = *(UINT16*)ptr;
					}
					else
					{
						GPR[(int)EGPR::RSP] -= 8;
						*(UINT64*)GPR[(int)EGPR::RSP] = *(UINT64*)ptr;
					}
					status = true;
				}
			}break;
			};
		}break;
		}
	}break;
	default:
		break;
	}

	if (status)
		printf("PUSH\n");

	return status;
}

bool AssemblyState::service_sub()
{
	bool status = false;

	switch (*RIP)
	{
	case 0x28:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::REG_TO_REG:
		{
			auto idx_reg = modrm->RegisterMemory;
			auto idx_regmem = modrm->Register;
			if (Prefix.R)
				idx_reg += 8;
			if (Prefix.B)
				idx_regmem += 8;

			auto src = *(UINT8*)&GPR[idx_regmem];
			auto dest = *(UINT8*)&GPR[idx_reg];

			auto result = dest - src;

			*(UINT8*)&GPR[idx_reg] = result;

			FLAGS.SF = (result & 0x80) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (dest < src);
			FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80;

			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x29:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)
				{
					auto src = GPR[modrm_register];
					auto dest = *(UINT64*)ptr;

					auto result = dest - src;

					*(UINT64*)ptr = result;

					FLAGS.SF = (result & 0x8000000000000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000000000000000;
				}
				else if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)&GPR[modrm_register];
					auto dest = *(UINT16*)ptr;

					auto result = dest - src;

					*(UINT16*)ptr = result;

					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000;
				}
				else
				{
					auto src = *(UINT32*)&GPR[modrm_register];
					auto dest = *(UINT32*)ptr;

					auto result = dest - src;

					*(UINT32*)ptr = result;

					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80000000;
				}
				RIP += 2;
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto src = GPR[modrm_register];
				auto dest = GPR[modrm_register_memory];

				auto result = dest - src;

				GPR[modrm_register_memory] = result;

				FLAGS.SF = (result & 0x8000000000000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (dest < src);
				FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000000000000000;
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)&GPR[modrm_register];
					auto dest = *(UINT16*)&GPR[modrm_register_memory];
					auto result = dest - src;
					*(UINT16*)&GPR[modrm_register_memory] = result;
					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000;
				}
				else
				{
					auto src = *(UINT32*)&GPR[modrm_register];
					auto dest = *(UINT32*)&GPR[modrm_register_memory];
					auto result = dest - src;
					*(UINT32*)&GPR[modrm_register_memory] = result;
					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80000000;
				}
			}
			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x2A:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		log_ModRM(modrm);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

				auto src = *(UINT8*)ptr;
				auto dest = *(UINT8*)&GPR[modrm_register];
				auto result = dest - src;

				*(UINT8*)&GPR[modrm_register] = result;

				FLAGS.SF = (result & 0x80) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (dest < src);
				FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80;

				RIP += 2;
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			auto src = *(UINT8*)&GPR[modrm_register_memory];
			auto dest = *(UINT8*)&GPR[modrm_register];
			auto result = dest - src;

			*(UINT8*)&GPR[modrm_register] = result;

			FLAGS.SF = (result & 0x80) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (dest < src);
			FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80;

			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x2B:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

				if (Prefix.W)
				{
					auto src = *(UINT64*)ptr;
					auto dest = GPR[modrm_register];

					auto result = dest - src;
					GPR[modrm_register] = result;

					FLAGS.SF = (result & 0x8000000000000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000000000000000;
				}
				else if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)ptr;
					auto dest = *(UINT16*)&GPR[modrm_register];

					auto result = dest - src;
					*(UINT16*)&GPR[modrm_register] = result;

					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000;
				}
				else
				{
					auto src = *(UINT32*)ptr;
					auto dest = *(UINT32*)&GPR[modrm_register];

					auto result = dest - src;
					*(UINT32*)&GPR[modrm_register] = result;

					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80000000;
				}

				RIP += 2;
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto src = *(UINT64*)&GPR[modrm_register_memory];
				auto dest = GPR[modrm_register];

				auto result = dest - src;

				GPR[modrm_register] = result;

				FLAGS.SF = (result & 0x8000000000000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (dest < src);
				FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000000000000000;
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)&GPR[modrm_register_memory];
					auto dest = *(UINT16*)&GPR[modrm_register];
					auto result = dest - src;
					*(UINT16*)&GPR[modrm_register] = result;
					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000;
				}
				else
				{
					auto src = *(UINT32*)&GPR[modrm_register_memory];
					auto dest = *(UINT32*)&GPR[modrm_register];
					auto result = dest - src;
					*(UINT32*)&GPR[modrm_register] = result;
					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80000000;
				}
			}
			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x2C:
	{
		auto imm = *(UINT8*)(&RIP[1]);

		auto src = imm;
		auto dest = *(UINT8*)&GPR[(int)EGPR::RAX];

		auto result = dest - src;
		*(UINT8*)&GPR[(int)EGPR::RAX] = result;

		FLAGS.SF = (result & 0x80) != 0;
		FLAGS.ZF = (result == 0);
		
		auto p = (UINT8)result;
		p ^= p >> 4;
		p ^= p >> 2;
		p ^= p >> 1;

		FLAGS.PF = (p & 1) == 0;
		FLAGS.CF = (dest < src);
		FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80;

		RIP += 2;
		status = true;
	}break;
	case 0x2D:
	{
		auto imm = *(UINT32*)(&RIP[1]);
		if (Prefix.OperandSize)
		{
			auto src = imm & 0xFFFF;
			auto dest = *(UINT16*)&GPR[(int)EGPR::RAX];

			auto result = dest - src;
			*(UINT16*)&GPR[(int)EGPR::RAX] = result;

			FLAGS.SF = (result & 0x8000) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (dest < src);
			FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000;
			
			RIP += 5;
			status = true;
		}
		else
		{
			auto src = imm;
			auto dest = *(UINT32*)&GPR[(int)EGPR::RAX];

			auto result = dest - src;
			*(UINT32*)&GPR[(int)EGPR::RAX] = result;

			FLAGS.SF = (result & 0x80000000) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (dest < src);
			FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80000000;

			RIP += 5;
			status = true;
		}
	}break;
	case 0x80:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 5:
		{
			auto imm = *(UINT8*)(&RIP[2]);
			auto idx_reg = modrm->RegisterMemory;
			if (Prefix.B)
				idx_reg += 8;
			
			auto src = imm;
			auto dest = *(UINT8*)&GPR[idx_reg];

			auto result = dest - src;
			*(UINT8*)&GPR[idx_reg] = result;

			FLAGS.SF = (result & 0x80) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (dest < src);
			FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80;

			RIP += 3;
			status = true;
		}break;
		};
	}break;
	case 0x81:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 5:
		{
			auto imm = *(UINT32*)(&RIP[2]);
			auto idx_reg = modrm->RegisterMemory;
			if (Prefix.B)
				idx_reg += 8;
			
			auto src = imm;
			auto dest = GPR[idx_reg];

			auto result = dest - src;
			GPR[idx_reg] = result;

			FLAGS.SF = (result & 0x8000000000000000) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (dest < src);
			FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000000000000000;

			RIP += 6;
			status = true;
		}break;
		};
	}break;
	case 0x83:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 5:
		{
			auto imm_byte = *(INT8*)(&RIP[2]);
			auto idx_reg = modrm->RegisterMemory;
			if (Prefix.B)
				idx_reg += 8;

			if (Prefix.W)
			{
				auto src = (UINT64)(INT64)imm_byte;
				auto dest = GPR[idx_reg];

				auto result = dest - src;
				GPR[idx_reg] = result;

				FLAGS.SF = (result & 0x8000000000000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (dest < src);
				FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000000000000000;
			}
			else if (Prefix.OperandSize)
			{
				auto src = (UINT16)(INT16)imm_byte;
				auto dest = *(UINT16*)&GPR[idx_reg];

				auto result = dest - src;
				*(UINT16*)&GPR[idx_reg] = result;

				FLAGS.SF = (result & 0x8000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (dest < src);
				FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000;
			}
			else
			{
				auto src = (UINT32)(INT32)imm_byte;
				auto dest = *(UINT32*)&GPR[idx_reg];

				auto result = dest - src;
				*(UINT32*)&GPR[idx_reg] = result;

				FLAGS.SF = (result & 0x80000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (dest < src);
				FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80000000;
			}

			RIP += 3;
			status = true;
		}break;
		};
	}break;
	};

	if (status)
		printf("SUB\n");

	return status;
}

bool AssemblyState::service_add()
{
	bool status = false;

	switch (*RIP)
	{
	case 0x00:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::REG_TO_REG:
		{
			auto idx_reg = modrm->RegisterMemory;
			auto idx_regmem = modrm->Register;
			if (Prefix.R)
				idx_reg += 8;
			if (Prefix.B)
				idx_regmem += 8;

			auto src = *(UINT8*)&GPR[idx_regmem];
			auto dest = *(UINT8*)&GPR[idx_reg];

			auto result = dest + src;

			*(UINT8*)&GPR[idx_reg] = result;

			FLAGS.SF = (result & 0x80) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (result < dest);
			FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x80;

			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x01:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)
				{
					auto src = GPR[modrm_register];
					auto dest = *(UINT64*)ptr;

					auto result = dest + src;

					*(UINT64*)ptr = result;

					FLAGS.SF = (result & 0x8000000000000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (result < dest);
					FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x8000000000000000;
				}
				else if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)&GPR[modrm_register];
					auto dest = *(UINT16*)ptr;

					auto result = dest + src;

					*(UINT16*)ptr = result;

					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (result < dest);
					FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x8000;
				}
				else
				{
					auto src = *(UINT32*)&GPR[modrm_register];
					auto dest = *(UINT32*)ptr;

					auto result = dest + src;

					*(UINT32*)ptr = result;

					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (result < dest);
					FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x80000000;
				}
				RIP += 2;
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto src = GPR[modrm_register];
				auto dest = GPR[modrm_register_memory];

				auto result = dest + src;

				GPR[modrm_register_memory] = result;

				FLAGS.SF = (result & 0x8000000000000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (result < dest);
				FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x8000000000000000;
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)&GPR[modrm_register];
					auto dest = *(UINT16*)&GPR[modrm_register_memory];
					auto result = dest + src;
					*(UINT16*)&GPR[modrm_register_memory] = result;
					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (result < dest);
					FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x8000;
				}
				else
				{
					auto src = *(UINT32*)&GPR[modrm_register];
					auto dest = *(UINT32*)&GPR[modrm_register_memory];
					auto result = dest + src;
					*(UINT32*)&GPR[modrm_register_memory] = result;
					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (result < dest);
					FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x80000000;
				}
			}
			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x02:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		log_ModRM(modrm);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

				auto src = *(UINT8*)ptr;
				auto dest = *(UINT8*)&GPR[modrm_register];
				auto result = dest + src;

				*(UINT8*)&GPR[modrm_register] = result;

				FLAGS.SF = (result & 0x80) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (result < dest);
				FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x80;

				RIP += 2;
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			auto src = *(UINT8*)&GPR[modrm_register_memory];
			auto dest = *(UINT8*)&GPR[modrm_register];
			auto result = dest + src;

			*(UINT8*)&GPR[modrm_register] = result;

			FLAGS.SF = (result & 0x80) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (result < dest);
			FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x80;

			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x03:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

				if (Prefix.W)
				{
					auto src = *(UINT64*)ptr;
					auto dest = GPR[modrm_register];

					auto result = dest + src;
					GPR[modrm_register] = result;

					FLAGS.SF = (result & 0x8000000000000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (result < dest);
					FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x8000000000000000;
				}
				else if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)ptr;
					auto dest = *(UINT16*)&GPR[modrm_register];

					auto result = dest + src;
					*(UINT16*)&GPR[modrm_register] = result;

					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (result < dest);
					FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x8000;
				}
				else
				{
					auto src = *(UINT32*)ptr;
					auto dest = *(UINT32*)&GPR[modrm_register];

					auto result = dest + src;
					*(UINT32*)&GPR[modrm_register] = result;

					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (result < dest);
					FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x80000000;
				}

				RIP += 2;
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto src = *(UINT64*)&GPR[modrm_register_memory];
				auto dest = GPR[modrm_register];

				auto result = dest + src;

				GPR[modrm_register] = result;

				FLAGS.SF = (result & 0x8000000000000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (result < dest);
				FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x8000000000000000;
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)&GPR[modrm_register_memory];
					auto dest = *(UINT16*)&GPR[modrm_register];
					auto result = dest + src;
					*(UINT16*)&GPR[modrm_register] = result;
					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (result < dest);
					FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x8000;
				}
				else
				{
					auto src = *(UINT32*)&GPR[modrm_register_memory];
					auto dest = *(UINT32*)&GPR[modrm_register];
					auto result = dest + src;
					*(UINT32*)&GPR[modrm_register] = result;
					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (result < dest);
					FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x80000000;
				}
			}
			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x04:
	{
		auto imm = *(UINT8*)(&RIP[1]);

		auto src = imm;
		auto dest = *(UINT8*)&GPR[(int)EGPR::RAX];

		auto result = dest + src;
		*(UINT8*)&GPR[(int)EGPR::RAX] = result;

		FLAGS.SF = (result & 0x80) != 0;
		FLAGS.ZF = (result == 0);

		auto p = (UINT8)result;
		p ^= p >> 4;
		p ^= p >> 2;
		p ^= p >> 1;

		FLAGS.PF = (p & 1) == 0;
		FLAGS.CF = (result < dest);
		FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x80;

		RIP += 2;
		status = true;
	}break;
	case 0x05:
	{
		auto imm = *(UINT32*)(&RIP[1]);
		if (Prefix.OperandSize)
		{
			auto src = imm & 0xFFFF;
			auto dest = *(UINT16*)&GPR[(int)EGPR::RAX];

			auto result = dest + src;
			*(UINT16*)&GPR[(int)EGPR::RAX] = result;

			FLAGS.SF = (result & 0x8000) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (result < dest);
			FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x8000;

			RIP += 5;
			status = true;
		}
		else
		{
			auto src = imm;
			auto dest = *(UINT32*)&GPR[(int)EGPR::RAX];

			auto result = dest + src;
			*(UINT32*)&GPR[(int)EGPR::RAX] = result;

			FLAGS.SF = (result & 0x80000000) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (result < dest);
			FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x80000000;

			RIP += 5;
			status = true;
		}
	}break;
	case 0x80:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		{
			auto imm = *(UINT8*)(&RIP[2]);
			auto idx_reg = modrm->RegisterMemory;
			if (Prefix.B)
				idx_reg += 8;

			auto src = imm;
			auto dest = *(UINT8*)&GPR[idx_reg];

			auto result = dest + src;
			*(UINT8*)&GPR[idx_reg] = result;

			FLAGS.SF = (result & 0x80) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (result < dest);
			FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x80;

			RIP += 3;
			status = true;
		}break;
		};
	}break;
	case 0x81:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		{
			auto imm = *(UINT32*)(&RIP[2]);
			auto idx_reg = modrm->RegisterMemory;
			if (Prefix.B)
				idx_reg += 8;

			auto src = imm;
			auto dest = GPR[idx_reg];

			auto result = dest + src;
			GPR[idx_reg] = result;

			FLAGS.SF = (result & 0x8000000000000000) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (result < dest);
			FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x8000000000000000;

			RIP += 6;
			status = true;
		}break;
		};
	}break;
	case 0x83:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		{
			auto imm_byte = *(INT8*)(&RIP[2]);
			auto idx_reg = modrm->RegisterMemory;
			if (Prefix.B)
				idx_reg += 8;

			if (Prefix.W)
			{
				auto src = (UINT64)(INT64)imm_byte;
				auto dest = GPR[idx_reg];

				auto result = dest + src;
				GPR[idx_reg] = result;

				FLAGS.SF = (result & 0x8000000000000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (result < dest);
				FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x8000000000000000;
			}
			else if (Prefix.OperandSize)
			{
				auto src = (UINT16)(INT16)imm_byte;
				auto dest = *(UINT16*)&GPR[idx_reg];

				auto result = dest + src;
				*(UINT16*)&GPR[idx_reg] = result;

				FLAGS.SF = (result & 0x8000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (result < dest);
				FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x8000;
			}
			else
			{
				auto src = (UINT32)(INT32)imm_byte;
				auto dest = *(UINT32*)&GPR[idx_reg];

				auto result = dest + src;
				*(UINT32*)&GPR[idx_reg] = result;

				FLAGS.SF = (result & 0x80000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (result < dest);
				FLAGS.OF = ((~(dest ^ src)) & (dest ^ result)) & 0x80000000;
			}

			RIP += 3;
			status = true;
		}break;
		};
	}break;
	};

	if (status)
		printf("ADD\n");

	return status;
}

bool AssemblyState::service_xor()
{
	bool status = false;

	switch (*RIP)
	{
	case 0x30:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::REG_TO_REG:
		{
			auto idx_reg = modrm->RegisterMemory;
			auto idx_regmem = modrm->Register;
			if (Prefix.R)
				idx_reg += 8;
			if (Prefix.B)
				idx_regmem += 8;

			auto src = *(UINT8*)&GPR[idx_regmem];
			auto dest = *(UINT8*)&GPR[idx_reg];

			auto result = dest ^ src;

			*(UINT8*)&GPR[idx_reg] = result;

			FLAGS.SF = (result & 0x80) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = 0;
			FLAGS.OF = 0;

			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x31:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)
				{
					auto src = GPR[modrm_register];
					auto dest = *(UINT64*)ptr;

					auto result = dest ^ src;

					*(UINT64*)ptr = result;

					FLAGS.SF = (result & 0x8000000000000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
				}
				else if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)&GPR[modrm_register];
					auto dest = *(UINT16*)ptr;

					auto result = dest ^ src;

					*(UINT16*)ptr = result;

					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
				}
				else
				{
					auto src = *(UINT32*)&GPR[modrm_register];
					auto dest = *(UINT32*)ptr;

					auto result = dest ^ src;

					*(UINT32*)ptr = result;

					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
				}
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto src = GPR[modrm_register];
				auto dest = GPR[modrm_register_memory];

				auto result = dest ^ src;

				GPR[modrm_register_memory] = result;

				FLAGS.SF = (result & 0x8000000000000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = 0;
				FLAGS.OF = 0;
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)&GPR[modrm_register];
					auto dest = *(UINT16*)&GPR[modrm_register_memory];
					auto result = dest ^ src;
					*(UINT16*)&GPR[modrm_register_memory] = result;
					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
				}
				else
				{
					auto src = *(UINT32*)&GPR[modrm_register];
					auto dest = *(UINT32*)&GPR[modrm_register_memory];
					auto result = dest ^ src;
					*(UINT32*)&GPR[modrm_register_memory] = result;
					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
				}
			}
			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x32:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		log_ModRM(modrm);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

				auto src = *(UINT8*)ptr;
				auto dest = *(UINT8*)&GPR[modrm_register];
				auto result = dest ^ src;

				*(UINT8*)&GPR[modrm_register] = result;

				FLAGS.SF = (result & 0x80) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = 0;
				FLAGS.OF = 0;

				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			auto src = *(UINT8*)&GPR[modrm_register_memory];
			auto dest = *(UINT8*)&GPR[modrm_register];
			auto result = dest ^ src;

			*(UINT8*)&GPR[modrm_register] = result;

			FLAGS.SF = (result & 0x80) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = 0;
			FLAGS.OF = 0;

			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x33:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

				if (Prefix.W)
				{
					auto src = *(UINT64*)ptr;
					auto dest = GPR[modrm_register];

					auto result = dest ^ src;
					GPR[modrm_register] = result;

					FLAGS.SF = (result & 0x8000000000000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
				}
				else if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)ptr;
					auto dest = *(UINT16*)&GPR[modrm_register];

					auto result = dest ^ src;
					*(UINT16*)&GPR[modrm_register] = result;

					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
				}
				else
				{
					auto src = *(UINT32*)ptr;
					auto dest = *(UINT32*)&GPR[modrm_register];

					auto result = dest ^ src;
					*(UINT32*)&GPR[modrm_register] = result;

					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
				}

				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto src = *(UINT64*)&GPR[modrm_register_memory];
				auto dest = GPR[modrm_register];

				auto result = dest ^ src;

				GPR[modrm_register] = result;

				FLAGS.SF = (result & 0x8000000000000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = 0;
				FLAGS.OF = 0;
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)&GPR[modrm_register_memory];
					auto dest = *(UINT16*)&GPR[modrm_register];
					auto result = dest ^ src;
					*(UINT16*)&GPR[modrm_register] = result;
					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
				}
				else
				{
					auto src = *(UINT32*)&GPR[modrm_register_memory];
					auto dest = *(UINT32*)&GPR[modrm_register];
					auto result = dest ^ src;
					*(UINT32*)&GPR[modrm_register] = result;
					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
				}
			}
			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x34:
	{
		auto imm = *(UINT8*)(&RIP[1]);

		auto src = imm;
		auto dest = *(UINT8*)&GPR[(int)EGPR::RAX];

		auto result = dest ^ src;
		*(UINT8*)&GPR[(int)EGPR::RAX] = result;

		FLAGS.SF = (result & 0x80) != 0;
		FLAGS.ZF = (result == 0);

		auto p = (UINT8)result;
		p ^= p >> 4;
		p ^= p >> 2;
		p ^= p >> 1;

		FLAGS.PF = (p & 1) == 0;
		FLAGS.CF = 0;
		FLAGS.OF = 0;

		RIP += 2;
		status = true;
	}break;
	case 0x35:
	{
		auto imm = *(UINT32*)(&RIP[1]);
		if (Prefix.OperandSize)
		{
			auto src = imm & 0xFFFF;
			auto dest = *(UINT16*)&GPR[(int)EGPR::RAX];

			auto result = dest ^ src;
			*(UINT16*)&GPR[(int)EGPR::RAX] = result;

			FLAGS.SF = (result & 0x8000) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = 0;
			FLAGS.OF = 0;

			RIP += 5;
			status = true;
		}
		else
		{
			auto src = imm;
			auto dest = *(UINT32*)&GPR[(int)EGPR::RAX];

			auto result = dest ^ src;
			*(UINT32*)&GPR[(int)EGPR::RAX] = result;

			FLAGS.SF = (result & 0x80000000) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = 0;
			FLAGS.OF = 0;

			RIP += 5;
			status = true;
		}
	}break;
	case 0x80:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 6:
		{
			auto imm = *(UINT8*)(&RIP[2]);
			auto idx_reg = modrm->RegisterMemory;
			if (Prefix.B)
				idx_reg += 8;

			auto src = imm;
			auto dest = *(UINT8*)&GPR[idx_reg];

			auto result = dest ^ src;
			*(UINT8*)&GPR[idx_reg] = result;

			FLAGS.SF = (result & 0x80) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = 0;
			FLAGS.OF = 0;

			RIP += 3;
			status = true;
		}break;
		};
	}break;
	case 0x81:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 6:
		{
			auto imm = *(UINT32*)(&RIP[2]);
			auto idx_reg = modrm->RegisterMemory;
			if (Prefix.B)
				idx_reg += 8;

			auto src = imm;
			auto dest = GPR[idx_reg];

			auto result = dest ^ src;
			GPR[idx_reg] = result;

			FLAGS.SF = (result & 0x8000000000000000) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = 0;
			FLAGS.OF = 0;

			RIP += 6;
			status = true;
		}break;
		};
	}break;
	case 0x83:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 6:
		{

			auto imm = *(UINT8*)(&RIP[2]);
			auto idx_reg = modrm->RegisterMemory;
			if (Prefix.B)
				idx_reg += 8;

			auto src = imm;
			auto dest = GPR[idx_reg];

			auto result = dest ^ src;
			GPR[idx_reg] = result;

			FLAGS.SF = (result & 0x8000000000000000) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = 0;
			FLAGS.OF = 0;

			RIP += 3;
			status = true;
		}break;
		};
	}break;
	};

	if (status)
		printf("XOR\n");

	return status;
}

bool AssemblyState::service_test()
{
	bool status = false;

	switch (*RIP)
	{
	case 0x84:
	{
		//r/m8, r8
		auto modrm = (MODRM*)(&RIP[1]);

		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

				auto src = *(UINT8*)ptr;
				auto dest = *(UINT8*)&GPR[modrm_register];
				auto temp = src & dest;

				FLAGS.SF = (temp & 0x80) != 0;
				FLAGS.ZF = (temp == 0);

				auto byte = (UINT8)temp;
				byte ^= byte >> 4;
				byte ^= byte >> 2;
				byte ^= byte >> 1;
				FLAGS.PF = (byte & 1) == 0;
				FLAGS.CF = 0;
				FLAGS.OF = 0;
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
			auto src = *(UINT8*)&GPR[modrm_register_memory];
			auto dest = *(UINT8*)&GPR[modrm_register];
			auto temp = src & dest;
			FLAGS.SF = (temp & 0x80) != 0;
			FLAGS.ZF = (temp == 0);
			auto byte = (UINT8)temp;
			byte ^= byte >> 4;
			byte ^= byte >> 2;
			byte ^= byte >> 1;
			FLAGS.PF = (byte & 1) == 0;
			FLAGS.CF = 0;
			FLAGS.OF = 0;
			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x85:
	{
		//r/m16/32/64, r16/32/64
		auto modrm = (MODRM*)(&RIP[1]);

		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)//64 bits
				{
					auto src = *(UINT64*)ptr;
					auto dest = GPR[modrm_register];
					auto temp = src & dest;
					FLAGS.SF = (temp & 0x8000000000000000) != 0;
					FLAGS.ZF = (temp == 0);
					auto byte = (UINT8)temp;
					byte ^= byte >> 4;
					byte ^= byte >> 2;
					byte ^= byte >> 1;
					FLAGS.PF = (byte & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
					status = true;
				}
				else
				{
					if (Prefix.OperandSize)//16 bits
					{
						auto src = *(UINT16*)ptr;
						auto dest = *(UINT16*)&GPR[modrm_register];
						auto temp = src & dest;
						FLAGS.SF = (temp & 0x8000) != 0;
						FLAGS.ZF = (temp == 0);
						auto byte = (UINT8)temp;
						byte ^= byte >> 4;
						byte ^= byte >> 2;
						byte ^= byte >> 1;
						FLAGS.PF = (byte & 1) == 0;
						FLAGS.CF = 0;
						FLAGS.OF = 0;
						status = true;
					}
					else// 32 bits
					{
						auto src = *(UINT32*)ptr;
						auto dest = *(UINT32*)&GPR[modrm_register];
						auto temp = src & dest;
						FLAGS.SF = (temp & 0x80000000) != 0;
						FLAGS.ZF = (temp == 0);
						auto byte = (UINT8)temp;
						byte ^= byte >> 4;
						byte ^= byte >> 2;
						byte ^= byte >> 1;
						FLAGS.PF = (byte & 1) == 0;
						FLAGS.CF = 0;
						FLAGS.OF = 0;
						status = true;
					}
				}
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
			if (Prefix.W)//64 bits
			{
				auto src = GPR[modrm_register_memory];
				auto dest = GPR[modrm_register];
				auto temp = src & dest;
				FLAGS.SF = (temp & 0x8000000000000000) != 0;
				FLAGS.ZF = (temp == 0);
				auto byte = (UINT8)temp;
				byte ^= byte >> 4;
				byte ^= byte >> 2;
				byte ^= byte >> 1;
				FLAGS.PF = (byte & 1) == 0;
				FLAGS.CF = 0;
				FLAGS.OF = 0;
				RIP += 2;
				status = true;
			}
			else
			{
				if (Prefix.OperandSize)//16 bits
				{
					auto src = *(UINT16*)&GPR[modrm_register_memory];
					auto dest = *(UINT16*)&GPR[modrm_register];
					auto temp = src & dest;
					FLAGS.SF = (temp & 0x8000) != 0;
					FLAGS.ZF = (temp == 0);
					auto byte = (UINT8)temp;
					byte ^= byte >> 4;
					byte ^= byte >> 2;
					byte ^= byte >> 1;
					FLAGS.PF = (byte & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
					RIP += 2;
					status = true;
				}
				else//32 bits
				{
					auto src = *(UINT32*)&GPR[modrm_register_memory];
					auto dest = *(UINT32*)&GPR[modrm_register];
					auto temp = src & dest;
					FLAGS.SF = (temp & 0x80000000) != 0;
					FLAGS.ZF = (temp == 0);
					auto byte = (UINT8)temp;
					byte ^= byte >> 4;
					byte ^= byte >> 2;
					byte ^= byte >> 1;
					FLAGS.PF = (byte & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
					RIP += 2;
					status = true;
				}
			}
		}break;
		};
	}break;
	case 0xA8:
	{
		//Al, imm8
		auto imm = *(UINT8*)(&RIP[1]);
		auto src = imm;
		auto dest = *(UINT8*)&GPR[(int)EGPR::RAX];
		auto temp = src & dest;
		FLAGS.SF = (temp & 0x80) != 0;
		FLAGS.ZF = (temp == 0);
		auto byte = (UINT8)temp;
		byte ^= byte >> 4;
		byte ^= byte >> 2;
		byte ^= byte >> 1;
		FLAGS.PF = (byte & 1) == 0;
		FLAGS.CF = 0;
		FLAGS.OF = 0;
		RIP += 2;
		status = true;
	}break;
	case 0xA9:
	{
		//rAX, imm16/32
		auto imm = *(UINT32*)(&RIP[1]);
		if (Prefix.OperandSize)
		{
			auto src = imm & 0xFFFF;
			auto dest = *(UINT16*)&GPR[(int)EGPR::RAX];
			auto temp = src & dest;
			FLAGS.SF = (temp & 0x8000) != 0;
			FLAGS.ZF = (temp == 0);
			auto byte = (UINT8)temp;
			byte ^= byte >> 4;
			byte ^= byte >> 2;
			byte ^= byte >> 1;
			FLAGS.PF = (byte & 1) == 0;
			FLAGS.CF = 0;
			FLAGS.OF = 0;
			RIP += 5;
			status = true;
		}
		else
		{
			auto src = imm;
			auto dest = *(UINT32*)&GPR[(int)EGPR::RAX];
			auto temp = src & dest;
			FLAGS.SF = (temp & 0x80000000) != 0;
			FLAGS.ZF = (temp == 0);
			auto byte = (UINT8)temp;
			byte ^= byte >> 4;
			byte ^= byte >> 2;
			byte ^= byte >> 1;
			FLAGS.PF = (byte & 1) == 0;
			FLAGS.CF = 0;
			FLAGS.OF = 0;
			RIP += 5;
			status = true;
		}
	}break;
	case 0xF6:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		case 1:
		{
			//r/m8, imm8
			auto modrm_registermemory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
			auto imm = *(UINT8*)(&RIP[2]);
			auto src = *(UINT8*)&GPR[modrm_registermemory];
			auto dest = imm;
			auto temp = src & dest;
			FLAGS.SF = (temp & 0x80) != 0;
			FLAGS.ZF = (temp == 0);
			auto byte = (UINT8)temp;
			byte ^= byte >> 4;
			byte ^= byte >> 2;
			byte ^= byte >> 1;
			FLAGS.PF = (byte & 1) == 0;
			FLAGS.CF = 0;
			FLAGS.OF = 0;
			RIP += 3;
			status = true;
		}break;
		};
	}break;
	case 0xF7:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		case 1:
		{
			//r/m16/32/64, imm16/32
			auto modrm_registermemory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
			auto imm = *(UINT32*)(&RIP[2]);
			if (Prefix.W)
			{
				auto src = GPR[modrm_registermemory];
				auto dest = imm;
				auto temp = src & dest;
				FLAGS.SF = (temp & 0x8000000000000000) != 0;
				FLAGS.ZF = (temp == 0);
				auto byte = (UINT8)temp;
				byte ^= byte >> 4;
				byte ^= byte >> 2;
				byte ^= byte >> 1;
				FLAGS.PF = (byte & 1) == 0;
				FLAGS.CF = 0;
				FLAGS.OF = 0;
				RIP += 6;
				status = true;
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)&GPR[modrm_registermemory];
					auto dest = imm & 0xFFFF;
					auto temp = src & dest;
					FLAGS.SF = (temp & 0x8000) != 0;
					FLAGS.ZF = (temp == 0);
					auto byte = (UINT8)temp;
					byte ^= byte >> 4;
					byte ^= byte >> 2;
					byte ^= byte >> 1;
					FLAGS.PF = (byte & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
					RIP += 6;
					status = true;
				}
				else
				{
					auto src = *(UINT32*)&GPR[modrm_registermemory];
					auto dest = imm;
					auto temp = src & dest;
					FLAGS.SF = (temp & 0x80000000) != 0;
					FLAGS.ZF = (temp == 0);
					auto byte = (UINT8)temp;
					byte ^= byte >> 4;
					byte ^= byte >> 2;
					byte ^= byte >> 1;
					FLAGS.PF = (byte & 1) == 0;
					FLAGS.CF = 0;
					FLAGS.OF = 0;
					RIP += 6;
					status = true;
				}
			}
		}break;
		};
	}break;
	};

	if(status)
		printf("TEST\n");

	return status;
}

bool AssemblyState::service_cmp()
{
	bool status = false;

	switch (*RIP)
	{
	case 0x38:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::REG_TO_REG:
		{
			auto idx_reg = modrm->RegisterMemory;
			auto idx_regmem = modrm->Register;
			if (Prefix.R)
				idx_reg += 8;
			if (Prefix.B)
				idx_regmem += 8;

			auto src = *(UINT8*)&GPR[idx_regmem];
			auto dest = *(UINT8*)&GPR[idx_reg];

			auto result = dest - src;

			FLAGS.SF = (result & 0x80) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (dest < src);
			FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80;

			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x39:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)
				{
					auto src = GPR[modrm_register];
					auto dest = *(UINT64*)ptr;

					auto result = dest - src;

					FLAGS.SF = (result & 0x8000000000000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000000000000000;
				}
				else if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)&GPR[modrm_register];
					auto dest = *(UINT16*)ptr;

					auto result = dest - src;

					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000;
				}
				else
				{
					auto src = *(UINT32*)&GPR[modrm_register];
					auto dest = *(UINT32*)ptr;

					auto result = dest - src;

					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80000000;
				}
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto src = GPR[modrm_register];
				auto dest = GPR[modrm_register_memory];

				auto result = dest - src;

				FLAGS.SF = (result & 0x8000000000000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (dest < src);
				FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000000000000000;
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)&GPR[modrm_register];
					auto dest = *(UINT16*)&GPR[modrm_register_memory];
					auto result = dest - src;
					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000;
				}
				else
				{
					auto src = *(UINT32*)&GPR[modrm_register];
					auto dest = *(UINT32*)&GPR[modrm_register_memory];
					auto result = dest - src;
					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80000000;
				}
			}
			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x3A:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		log_ModRM(modrm);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

				auto src = *(UINT8*)ptr;
				auto dest = *(UINT8*)&GPR[modrm_register];
				auto result = dest - src;

				FLAGS.SF = (result & 0x80) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (dest < src);
				FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80;
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			auto src = *(UINT8*)&GPR[modrm_register_memory];
			auto dest = *(UINT8*)&GPR[modrm_register];
			auto result = dest - src;

			FLAGS.SF = (result & 0x80) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (dest < src);
			FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80;

			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x3B:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

				if (Prefix.W)
				{
					auto src = *(UINT64*)ptr;
					auto dest = GPR[modrm_register];

					auto result = dest - src;

					FLAGS.SF = (result & 0x8000000000000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000000000000000;
				}
				else if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)ptr;
					auto dest = *(UINT16*)&GPR[modrm_register];

					auto result = dest - src;

					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000;
				}
				else
				{
					auto src = *(UINT32*)ptr;
					auto dest = *(UINT32*)&GPR[modrm_register];

					auto result = dest - src;

					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);

					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80000000;
				}
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto src = *(UINT64*)&GPR[modrm_register_memory];
				auto dest = GPR[modrm_register];

				auto result = dest - src;

				FLAGS.SF = (result & 0x8000000000000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (dest < src);
				FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000000000000000;
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto src = *(UINT16*)&GPR[modrm_register_memory];
					auto dest = *(UINT16*)&GPR[modrm_register];
					auto result = dest - src;
					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000;
				}
				else
				{
					auto src = *(UINT32*)&GPR[modrm_register_memory];
					auto dest = *(UINT32*)&GPR[modrm_register];
					auto result = dest - src;
					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80000000;
				}
			}
			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x3C:
	{
		auto imm = *(UINT8*)(&RIP[1]);

		auto src = imm;
		auto dest = *(UINT8*)&GPR[(int)EGPR::RAX];

		auto result = dest - src;

		FLAGS.SF = (result & 0x80) != 0;
		FLAGS.ZF = (result == 0);

		auto p = (UINT8)result;
		p ^= p >> 4;
		p ^= p >> 2;
		p ^= p >> 1;

		FLAGS.PF = (p & 1) == 0;
		FLAGS.CF = (dest < src);
		FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80;

		RIP += 2;
		status = true;
	}break;
	case 0x3D:
	{
		auto imm = *(UINT32*)(&RIP[1]);
		if (Prefix.OperandSize)
		{
			auto src = imm & 0xFFFF;
			auto dest = *(UINT16*)&GPR[(int)EGPR::RAX];

			auto result = dest - src;

			FLAGS.SF = (result & 0x8000) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (dest < src);
			FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000;

			RIP += 5;
			status = true;
		}
		else
		{
			auto src = imm;
			auto dest = *(UINT32*)&GPR[(int)EGPR::RAX];

			auto result = dest - src;

			FLAGS.SF = (result & 0x80000000) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (dest < src);
			FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80000000;

			RIP += 5;
			status = true;
		}
	}break;
	case 0x80:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 7:
		{
			auto imm = *(UINT8*)(&RIP[2]);
			auto idx_reg = modrm->RegisterMemory;
			if (Prefix.B)
				idx_reg += 8;

			auto src = imm;
			auto dest = *(UINT8*)&GPR[idx_reg];

			auto result = dest - src;

			FLAGS.SF = (result & 0x80) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = (dest < src);
			FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80;

			RIP += 3;
			status = true;
		}break;
		};
	}break;
	case 0x81:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 7:
		{
			auto imm = *(UINT32*)(&RIP[2]);
			auto idx_reg = modrm->RegisterMemory;
			if (Prefix.B)
				idx_reg += 8;

			if (Prefix.W)
			{
				auto src = (UINT64)(INT64)(INT32)imm;
				auto dest = GPR[idx_reg];

				auto result = dest - src;

				FLAGS.SF = (result & 0x8000000000000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (dest < src);
				FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000000000000000;
			}
			else if (Prefix.OperandSize)
			{
				auto src = (UINT16)(INT16)(INT32)imm;
				auto dest = *(UINT16*)&GPR[idx_reg];

				auto result = dest - src;

				FLAGS.SF = (result & 0x8000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (dest < src);
				FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000;
			}
			else
			{
				auto src = (UINT32)(INT32)imm;
				auto dest = *(UINT32*)&GPR[idx_reg];

				auto result = dest - src;

				FLAGS.SF = (result & 0x80000000) != 0;
				FLAGS.ZF = (result == 0);

				auto p = (UINT8)result;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.CF = (dest < src);
				FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80000000;
			}

			RIP += 6;
			status = true;
		}break;
		};
	}break;
	case 0x83:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 7:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto imm_byte = (INT8)RIP[1];
				if (Prefix.W)
				{
					auto src = (UINT64)(INT64)imm_byte;
					auto dest = *(UINT64*)ptr;
				
					auto result = dest - src;
				
					FLAGS.SF = (result & 0x8000000000000000) != 0;
					FLAGS.ZF = (result == 0);
				
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
				
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000000000000000;
				}
				else if (Prefix.OperandSize)
				{
					auto src = (UINT16)(INT16)imm_byte;
					auto dest = *(UINT16*)ptr;
				
					auto result = dest - src;
				
					FLAGS.SF = (result & 0x8000) != 0;
					FLAGS.ZF = (result == 0);
				
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
				
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x8000;
				}
				else
				{
					auto src = (UINT32)(INT32)imm_byte;
					auto dest = *(UINT32*)ptr;
				
					auto result = dest - src;
				
					FLAGS.SF = (result & 0x80000000) != 0;
					FLAGS.ZF = (result == 0);
				
					auto p = (UINT8)result;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;
				
					FLAGS.PF = (p & 1) == 0;
					FLAGS.CF = (dest < src);
					FLAGS.OF = ((dest ^ src) & (dest ^ result)) & 0x80000000;
				}
				RIP += 1;
				status = true;
			}
			
		}break;
		};
	}break;
	}

	if (status)
		printf("CMP\n");

	return status;
}

bool AssemblyState::service_stosd()
{
	auto status = false;
	

	int count = 1;
	if (Prefix.Repeated)
		count = (int)GPR[(int)EGPR::RCX];

	for (int i = 0; i < count; i++)
	{
		auto ptr = GPR[(int)EGPR::RDI];

		if (Prefix.GS)
			ptr += GsBase;
		else if (Prefix.FS)
			ptr += FsBase;

		if (Prefix.OperandSize)
		{
			*(UINT16*)ptr = (UINT16)GPR[(int)EGPR::RAX];
			GPR[(int)EGPR::RDI] += 2;
		}
		else
		{
			*(UINT32*)ptr = (UINT32)GPR[(int)EGPR::RAX];
			GPR[(int)EGPR::RDI] += 4;
		}
	}
	
	status = true;
	RIP += 1;

	if(status)
		printf("STOSD\n");

	return status;
}

bool AssemblyState::decode_mnemonic()
{
	Advancement = 0;
	bool status = false;
	auto start_rip = RIP;
	Prefix = { 0 };

	// skip NOPs
	switch (*RIP)
	{
	case 0x90:
		RIP++;
		return true;
	};

	// repeated prefixes
	switch (*RIP)
	{
	case 0xF3:
	{
		RIP++;
		Prefix.Repeated = 1;
	}break;
	};

	// segment override prefixes
	switch (*RIP)
	{
	case 0x26:
	{
		Prefix.ES = 1;
		RIP++;
	}break;
	case 0x2E:
	{
		Prefix.CS = 1;
		RIP++;
	}break;
	case 0x36:
	{
		Prefix.SS = 1;
		RIP++;
	}break;
	case 0x3E:
	{
		Prefix.DS = 1;
		RIP++;
	}break;
	case 0x64:
	{
		Prefix.FS = 1;
		RIP++;
	}break;
	case 0x65:
	{
		Prefix.GS = 1;
		RIP++;
	}break;
	};

	// operand and address size override prefixes
	switch (*RIP)
	{
	case 0x66:
	{
		Prefix.OperandSize = 1;//32-16 bit
		RIP++;
	}break;
	case 0x67:
	{
		Prefix.AddressSize = 1;//64-32 bit
		RIP++;
	}break;
	};

	// REX prefix
	switch (*RIP)
	{
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
	case 0x48:
	case 0x49:
	case 0x4A:
	case 0x4B:
	case 0x4C:
	case 0x4D:
	case 0x4E:
	case 0x4F:
	{
		Prefix.W = (*RIP >> 3) & 1;
		Prefix.R = (*RIP >> 2) & 1;
		Prefix.X = (*RIP >> 1) & 1;
		Prefix.B = (*RIP >> 0) & 1;
		RIP++;
	}break;

	case 0x9B:
	{
		printf("Prefix 9B, Aborting\n");
		return false;
	}break;
	case 0xF0:
	{
		Prefix.LOCK = 1;
		RIP++;
	}break;
	case 0xF2:
	{
		printf("Prefix F2, Aborting\n");
		return false;
	}break;
	default:
		break;
	};

	// second pass for 0F prefix
	// third pass for primary opcode
	switch (*RIP)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	{
		status = service_add();
	}break;
	case 0x0F:
	{
		switch (RIP[1])
		{
		case 0xB6:
		{
			status = service_mov();
		}break;
		}
	}break;
	case 0x28:
	case 0x29:
	case 0x2A:
	case 0x2B:
	case 0x2C:
	case 0x2D:
	{
		status = service_sub();
	}break;
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	{
		status = service_xor();
	}break;
	case 0x38:
	case 0x39:
	case 0x3A:
	case 0x3B:
	case 0x3C:
	case 0x3D:
	{
		status = service_cmp();
	}break;
	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
	{
		status = service_push();
	}break;
	case 0x74:
	{
		status = service_jmp();
	}break;
	case 0x80:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		{
			status = service_add();
		}break;
		case 5:
		{
			status = service_sub();
		}break;
		case 6:
		{
			status = service_xor();
		}break;
		case 7:
		{
			status = service_cmp();
		}break;
		};
	}break;
	case 0x81:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		{
			status = service_add();
		}break;
		case 5:
		{
			status = service_sub();
		}break;
		case 6:
		{
			status = service_xor();
		}break;
		case 7:
		{
			status = service_cmp();
		}break;
		};
	}break;
	case 0x83:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		{
			status = service_add();
		}break;
		case 5:
		{
			status = service_sub();
		}break;
		case 6:
		{
			status = service_xor();
		}break;
		case 7:
		{
			status = service_cmp();
		}break;
		};
	}break;
	case 0x84:
	case 0x85:
	{
		status = service_test();
	}break;
	case 0x88:
	case 0x89:
	case 0x8A:
	case 0x8B:
	case 0x8C:
	{
		status = service_mov();
	}break;
	case 0x8D:
	{
		status = service_lea();
	}break;
	case 0x8E:
	{
		status = service_mov();
	}break;
	case 0xA0:
	case 0xA1:
	case 0xA2:
	case 0xA3:
	case 0xA4:
	case 0xA5:
	{
		status = service_mov();
	}break;
	case 0xA8:
	case 0xA9:
	{
		status = service_test();
	}break;
	case 0xAB:
	{
		status = service_stosd();
	}break;
	case 0xB0:
	case 0xB1:
	case 0xB2:
	case 0xB3:
	case 0xB4:
	case 0xB5:
	case 0xB6:
	case 0xB7:
	case 0xB8:
	case 0xB9:
	case 0xBA:
	case 0xBB:
	case 0xBC:
	case 0xBD:
	case 0xBE:
	case 0xBF:
	{
		status = service_mov();
	}break;
	case 0xC3:
	{
		status = service_ret();
	}break;
	case 0xC6:
	case 0xC7:
	{
		status = service_mov();
	}break;
	case 0xE8:
	{
		status = service_call();
	}break;
	case 0xE9:
	case 0xEB:
	{
		status = service_jmp();
	}break;
	case 0xF6:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		case 1:
		{
			status = service_test();
		}break;
		};
	}break;
	case 0xF7:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		case 1:
		{
			status = service_test();
		}break;
		};
	}break;
	case 0xFF:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 2:
		case 3:
		{
			status = service_call();
		}break;
		case 4:
		case 5:
		{
			status = service_jmp();
		}break;
		case 6:
		{
			status = service_push();
		}break;
		}
	}break;
	};

	if (status)
	{
		if ((INT64)start_rip < (INT64)RIP)
		{
			auto size = (UINT64)RIP - (UINT64)start_rip;
			if(Advancement && size > 10)
				size = Advancement - (UINT64)start_rip;
			else if(size > 10)
				size = 10;
			printf(" > Decoded [ ");
			for (UINT64 i = 0; i < size; i++)
				printf("%02X ", start_rip[i]);
			printf("]\n");
		}
		else
		{
			if (Advancement)
			{
				auto size = Advancement - (UINT64)start_rip;
				if (size > 10)
					size = 10;
				printf(" > Decoded [ ");
				for (UINT64 i = 0; i < size; i++)
					printf("%02X ", start_rip[i]);
				printf("]\n");
			}
			else
			{
				printf("\nFailed [ ");
				for (int i = 0; i < 10; i++)
					printf("%02X ", start_rip[i]);
				printf("]\n");
			}
		}
	}
	else
	{
		printf("\nFailed [ ");
		for (int i = 0; i < 10; i++)
			printf("%02X ", start_rip[i]);
		printf("]\n");
	}


	return status;
}

bool AssemblyState::step()
{
	bool status = false;

	status = decode_mnemonic();

	return status;
}

void test()
{
	printf("Hello world\n");
	return;
}

int main()
{
	auto engine = new AssemblyState();
	engine->SetGPR((int)EGPR::RSP, (UINT64)VirtualAlloc(nullptr, 0x10000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE) + 0x8000);
	engine->SetRip((PVOID)test);
	//auto code = "\x83\x3D\x76\x01\x00\x00\x00\x83\x3D\x17\x00\x00\x00\x00\x83\x7C\x24\x16\x00\x83\x3C\x24\x00\x83\x3C\xC4\x00\x83\x7D\x00\x00\x83\x3B\x00\x83\x38\x00\x83\xBC\x24\x76\x01\x00\x00\x00\x83\xB8\x76\x01\x00\x00\x00\x83\xBC\x04\x76\x01\x00\x00\x00\x83\xBC\xDC\x76\x01\x00\x00\x00";
	//engine->SetRip((PVOID)code);
	int counter = 0;
	while (engine->step())
	{
		counter++;
	}
	printf("Executed %d instructions\n", counter);

	return 0;
}

