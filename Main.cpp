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
};

class AssemblyState
{
private:
	UINT64 GPR[16];
	BYTE* RIP;
	EFLAGS FLAGS;
	UINT64 GsBase;
	UINT64 FsBase;
	MNEMONICPREFIX Prefix;

	void log_Prefix();
	void log_ModRM(MODRM* modrm);

	bool service_mov();
	bool service_jmp();
	bool service_push();
	bool service_pop();
	bool service_sub();
	bool decode_mnemonic();
public:
	void SetRip(PVOID rip);
	void SetGPR(int index, UINT64 value);
	bool step();
};

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
		{
			if (modrm->RegisterMemory == (int)EGPR::RSP)
			{
				auto modrm2 = (MODRM*)(&RIP[2]);

				auto mutiplier = 1ull << (BYTE)modrm2->Mode;
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				auto modrm2_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
				auto modrm2_register = Prefix.X ? modrm2->Register + 8 : modrm2->Register;

				if (modrm2->RegisterMemory == (BYTE)EGPR::RBP)
				{
					auto imm = *(INT32*)(&RIP[3]);
					auto ptr = imm + GPR[modrm2_register] * mutiplier;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					if (Prefix.W)
					{
						*(UINT64*)ptr = GPR[modrm_register];
					}
					else
					{
						*(UINT32*)ptr = *(UINT32*)&GPR[modrm_register];
					}
					RIP += 7;
					status = true;
				}
				else
				{
					auto ptr = GPR[modrm2_register_memory] + GPR[modrm2_register] * mutiplier;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					if (Prefix.W)
					{
						*(UINT64*)ptr = GPR[modrm_register];
					}
					else
					{
						*(UINT32*)ptr = *(UINT32*)&GPR[modrm_register];
					}
					RIP += 3;
					status = true;
				}
			}
			else
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

				auto ptr = GPR[modrm_register_memory];

				if (Prefix.GS)
					ptr += GsBase;
				else if (Prefix.FS)
					ptr += FsBase;

				if (Prefix.W)
				{
					*(UINT64*)ptr = GPR[modrm_register];
				}
				else
				{
					*(UINT32*)ptr = *(UINT32*)&GPR[modrm_register];
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
		{
			if (modrm->RegisterMemory == (int)EGPR::RSP)
			{
				auto modrm2 = (MODRM*)(&RIP[2]);

				auto mutiplier = 1ull << (BYTE)modrm2->Mode;
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				auto modrm2_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
				auto modrm2_register = Prefix.X ? modrm2->Register + 8 : modrm2->Register;

				if (modrm2->RegisterMemory == (BYTE)EGPR::RBP)
				{
					auto imm = *(INT32*)(&RIP[3]);
					auto ptr = imm + GPR[modrm2_register] * mutiplier;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					if (Prefix.W)
					{
						GPR[modrm_register] = *(UINT64*)ptr;
					}
					else
					{
						*(UINT32*)&GPR[modrm_register] = *(UINT32*)ptr;
					}
					RIP += 7;
					status = true;
				}
				else
				{
					auto ptr = GPR[modrm2_register_memory] + GPR[modrm2_register] * mutiplier;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					if (Prefix.W)
					{
						GPR[modrm_register] = *(UINT64*)ptr;
					}
					else
					{
						*(UINT32*)&GPR[modrm_register] = *(UINT32*)ptr;
					}
					RIP += 3;
					status = true;
				}
			}
			else
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

				auto ptr = GPR[modrm_register_memory];

				if (Prefix.GS)
					ptr += GsBase;
				else if (Prefix.FS)
					ptr += FsBase;

				if (Prefix.W)
				{
					GPR[modrm_register] = *(UINT64*)ptr;
				}
				else
				{
					*(UINT32*)&GPR[modrm_register] = *(UINT32*)ptr;
				}
				RIP += 2;
				status = true;
			}
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

bool AssemblyState::service_jmp()
{
	bool status = false;
	auto opcode = (BYTE*)RIP;
	switch (*opcode)
	{
	case 0xE9:
	{
		auto imm = *(INT32*)(opcode + 1);
		RIP += imm + 5;
		status = true;
	}break;
	case 0xEB:
	{
		auto imm = *(INT8*)(opcode + 1);
		RIP += imm + 2;
		status = true;
	}break;
	};

	if (status)
		printf("JUMP\n");

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
		
		*(UINT64*)GPR[(int)EGPR::RSP] = GPR[reg];
		RIP += 1;
		GPR[(int)EGPR::RSP] -= 8;
		status = true;
	}break;
	case 0xFF:
	{
		log_Prefix();
		auto modrm = (MODRM*)(&RIP[1]);
		log_ModRM(modrm);
		switch (modrm->Register)
		{
		case 6:
		{
			switch (modrm->Mode)
			{
			case EMODE::MEM_0_BIT_DISP:
			{
				if (modrm->RegisterMemory == (int)EGPR::RSP)
				{
					auto modrm2 = (MODRM*)(&RIP[2]);
					auto mutiplier = 1ull << (BYTE)modrm2->Mode;
					log_ModRM(modrm2);

					if (modrm2->RegisterMemory == (BYTE)EGPR::RBP)
					{
						
						auto idx_idx_regmem = modrm2->Register;
						if (Prefix.X)
							idx_idx_regmem += 8;
						auto imm = *(INT32*)(&RIP[3]);
						auto ptr = imm + GPR[idx_idx_regmem] * mutiplier;
						if (Prefix.GS)
							ptr += GsBase;
						else if (Prefix.FS)
							ptr += FsBase;
						if (Prefix.OperandSize)
						{
							*(UINT64*)GPR[(int)EGPR::RSP] = *(UINT64*)ptr;
							GPR[(int)EGPR::RSP] -= 8;
							RIP += 7;
							status = true;
						}
						else
						{
							*(UINT16*)GPR[(int)EGPR::RSP] = *(UINT16*)ptr;
							GPR[(int)EGPR::RSP] -= 2;
							RIP += 7;
							status = true;
						}
					}
					else
					{
						auto idx_reg = modrm2->RegisterMemory;
						if (Prefix.B)
							idx_reg += 8;
						auto idx_idx_regmem = modrm2->Register;
						if (Prefix.X)
							idx_idx_regmem += 8;
						auto ptr = GPR[idx_reg] + GPR[idx_idx_regmem] * mutiplier;
						if (Prefix.GS)
							ptr += GsBase;
						else if (Prefix.FS)
							ptr += FsBase;
						if (Prefix.OperandSize)
						{
							*(UINT64*)GPR[(int)EGPR::RSP] = *(UINT64*)ptr;
							GPR[(int)EGPR::RSP] -= 8;
							RIP += 3;
							status = true;
						}
						else
						{
							*(UINT16*)GPR[(int)EGPR::RSP] = *(UINT16*)ptr;
							GPR[(int)EGPR::RSP] -= 2;
							RIP += 3;
							status = true;
						}
					}
				}
				else
				{
					//push [rax]

					auto ptr = GPR[modrm->RegisterMemory];
					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;
					if (Prefix.OperandSize)
					{
						*(UINT64*)GPR[(int)EGPR::RSP] = *(UINT64*)ptr;
						GPR[(int)EGPR::RSP] -= 8;
						RIP += 2;
						status = true;
					}
					else
					{
						*(UINT16*)GPR[(int)EGPR::RSP] = *(UINT16*)ptr;
						GPR[(int)EGPR::RSP] -= 2;
						RIP += 2;
						status = true;
					}

				}
			}break;
			case EMODE::MEM_8_BIT_DISP:
			{
				auto idx_reg = modrm->RegisterMemory;
				if (Prefix.B)
					idx_reg += 8;
				auto imm = *(INT8*)(&RIP[2]);
				auto ptr = (INT64)GPR[idx_reg] + (INT64)imm;
				if(Prefix.GS)
					ptr += GsBase;
				else if (Prefix.FS)
					ptr += FsBase;

				if(Prefix.OperandSize)
				{
					*(UINT64*)GPR[(int)EGPR::RSP] = *(UINT64*)ptr;
					GPR[(int)EGPR::RSP] -= 8;
					RIP += 2;
					status = true;
				}
				else
				{
					*(UINT16*)GPR[(int)EGPR::RSP] = *(UINT16*)ptr;
					GPR[(int)EGPR::RSP] -= 2;
					RIP += 2;
					status = true;
				}
			}break;
			case EMODE::MEM_32_BIT_DISP:
			{
				auto idx_reg = modrm->RegisterMemory;
				if (Prefix.B)
					idx_reg += 8;
				auto imm = *(INT32*)(&RIP[2]);
				auto ptr = (INT64)GPR[idx_reg] + (INT64)imm;
				if (Prefix.GS)
					ptr += GsBase;
				else if (Prefix.FS)
					ptr += FsBase;

				if (Prefix.OperandSize)
				{
					*(UINT64*)GPR[(int)EGPR::RSP] = *(UINT64*)ptr;
					GPR[(int)EGPR::RSP] -= 8;
					RIP += 6;
					status = true;
				}
				else
				{
					*(UINT16*)GPR[(int)EGPR::RSP] = *(UINT16*)ptr;
					GPR[(int)EGPR::RSP] -= 2;
					RIP += 6;
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
			*(BYTE*)&GPR[idx_reg] -= *(BYTE*)&GPR[idx_regmem];
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
		{
			if (modrm->RegisterMemory == (int)EGPR::RSP)
			{
				auto modrm2 = (MODRM*)(&RIP[2]);

				auto mutiplier = 1ull << (BYTE)modrm2->Mode;
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				auto modrm2_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
				auto modrm2_register = Prefix.X ? modrm2->Register + 8 : modrm2->Register;

				if (modrm2->RegisterMemory == (BYTE)EGPR::RBP)
				{
					auto imm = *(INT32*)(&RIP[3]);
					auto ptr = imm + GPR[modrm2_register] * mutiplier;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					if (Prefix.W)
					{
						*(UINT64*)ptr -= GPR[modrm_register];
					}
					else
					{
						*(UINT32*)ptr -= *(UINT32*)&GPR[modrm_register];
					}
					RIP += 7;
					status = true;
				}
				else
				{
					auto ptr = GPR[modrm2_register_memory] + GPR[modrm2_register] * mutiplier;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					if (Prefix.W)
					{
						*(UINT64*)ptr -= GPR[modrm_register];
					}
					else
					{
						*(UINT32*)ptr -= *(UINT32*)&GPR[modrm_register];
					}
					RIP += 3;
					status = true;
				}
			}
			else
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

				auto ptr = GPR[modrm_register_memory];

				if (Prefix.GS)
					ptr += GsBase;
				else if (Prefix.FS)
					ptr += FsBase;

				if (Prefix.W)
				{
					*(UINT64*)ptr -= GPR[modrm_register];
				}
				else
				{
					*(UINT32*)ptr -= *(UINT32*)&GPR[modrm_register];
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
				GPR[modrm_register] -= GPR[modrm_register_memory];
			}
			else
			{
				if (Prefix.OperandSize)
				{
					*(UINT16*)&GPR[modrm_register] -= *(UINT16*)&GPR[modrm_register_memory];
				}
				else
				{
					*(UINT32*)&GPR[modrm_register] -= *(UINT32*)&GPR[modrm_register_memory];
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
		{
			if (modrm->RegisterMemory == (int)EGPR::RSP)
			{
				auto modrm2 = (MODRM*)(&RIP[2]);

				auto mutiplier = 1ull << (BYTE)modrm2->Mode;
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				auto modrm2_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
				auto modrm2_register = Prefix.X ? modrm2->Register + 8 : modrm2->Register;

				if (modrm2->RegisterMemory == (BYTE)EGPR::RBP)
				{
					auto imm = *(INT32*)(&RIP[3]);
					auto ptr = imm + GPR[modrm2_register] * mutiplier;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					*(BYTE*)&GPR[modrm_register] -= *(BYTE*)ptr;

					RIP += 7;
					status = true;
				}
				else
				{
					auto ptr = GPR[modrm2_register_memory] + GPR[modrm2_register] * mutiplier;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					*(BYTE*)&GPR[modrm_register] -= *(BYTE*)ptr;

					RIP += 3;
					status = true;
				}
			}
			else
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

				auto ptr = GPR[modrm_register_memory];

				if (Prefix.GS)
					ptr += GsBase;
				else if (Prefix.FS)
					ptr += FsBase;

				*(BYTE*)&GPR[modrm_register] -= *(BYTE*)ptr;

				RIP += 2;
				status = true;
			}
		}break;
		};
	}break;
	case 0x2B:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		{
			if (modrm->RegisterMemory == (int)EGPR::RSP)
			{
				auto modrm2 = (MODRM*)(&RIP[2]);

				auto mutiplier = 1ull << (BYTE)modrm2->Mode;
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				auto modrm2_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
				auto modrm2_register = Prefix.X ? modrm2->Register + 8 : modrm2->Register;

				if (modrm2->RegisterMemory == (BYTE)EGPR::RBP)
				{
					auto imm = *(INT32*)(&RIP[3]);
					auto ptr = imm + GPR[modrm2_register] * mutiplier;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					if (Prefix.W)
					{
						GPR[modrm_register] -= *(UINT64*)ptr;
					}
					else
					{
						*(UINT32*)&GPR[modrm_register] -= *(UINT32*)ptr;
					}
					RIP += 7;
					status = true;
				}
				else
				{
					auto ptr = GPR[modrm2_register_memory] + GPR[modrm2_register] * mutiplier;

					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					if (Prefix.W)
					{
						GPR[modrm_register] -= *(UINT64*)ptr;
					}
					else
					{
						*(UINT32*)&GPR[modrm_register] -= *(UINT32*)ptr;
					}
					RIP += 3;
					status = true;
				}
			}
			else
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

				auto ptr = GPR[modrm_register_memory];

				if (Prefix.GS)
					ptr += GsBase;
				else if (Prefix.FS)
					ptr += FsBase;

				if (Prefix.W)
				{
					GPR[modrm_register] -= *(UINT64*)ptr;
				}
				else
				{
					*(UINT32*)&GPR[modrm_register] -= *(UINT32*)ptr;
				}
				RIP += 2;
				status = true;
			}
		}break;
		};
	}break;
	case 0x2C:
	{
		auto imm = *(UINT8*)(&RIP[1]);
		*(UINT8*)&GPR[(int)EGPR::RAX] -= imm;
		RIP += 2;
		status = true;
	}break;
	case 0x2D:
	{
		auto imm = *(UINT32*)(&RIP[1]);
		if (Prefix.OperandSize)
		{
			*(UINT16*)&GPR[(int)EGPR::RAX] -= imm & 0xFFFF;
			RIP += 5;
			status = true;
		}
		else
		{
			*(UINT32*)&GPR[(int)EGPR::RAX] -= imm;
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
			*(UINT8*)&GPR[idx_reg] -= imm;
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
			GPR[idx_reg] -= imm;
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
			auto imm = *(UINT8*)(&RIP[2]);
			auto idx_reg = modrm->RegisterMemory;
			if (Prefix.B)
				idx_reg += 8;
			GPR[idx_reg] -= imm;
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

bool AssemblyState::decode_mnemonic()
{
	bool status = false;
	Prefix = { 0 };
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
	case 0xF3:
	{
		printf("Prefix F3, Aborting\n");
		return false;
	}break;
	default:
		break;
	}

	// second pass for 0F prefix
	if(*RIP == 0x0F)
	{
		RIP++;
		printf("Prefix 0F, Aborting\n");
		return false;
	}

	printf("Decoding [ ");
	for(int i=0;i<10;i++)
		printf("%02X ", RIP[i]);
	printf("]\n");
	// third pass for primary opcode
	switch (*RIP)
	{
	case 0x28:
	case 0x29:
	case 0x2A:
	case 0x2B:
	case 0x2C:
	case 0x2D:
	{
		status = service_sub();
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
	case 0x80:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 5:
		{
			status = service_sub();
		}break;
		}
	}break;
	case 0x81:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 5:
		{
			status = service_sub();
		}break;
		}
	}break;
	case 0x88:
	case 0x89:
	case 0x8A:
	case 0x8B:
	case 0x8C:
	case 0x8E:
	case 0xA0:
	case 0xA1:
	case 0xA2:
	case 0xA3:
	case 0xA4:
	case 0xA5:
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
	case 0xC6:
	case 0xC7:
	{
		status = service_mov();
	}break;
	case 0xE9:
	case 0xEB:
	{
		status = service_jmp();
	}break;
	case 0xFF:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 6:
		{
			status = service_push();
		}break;
		}
	}break;
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
	engine->SetGPR((int)EGPR::RSP, (UINT64)VirtualAlloc(nullptr, 0x10000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE) + 0xA000);
	engine->SetRip((PVOID)test);
	//auto code = "\x41\x29\x00\x48\x29\x04\xC0\x48\x29\x04\xC5\x00\x10\x00\x00\x48\x29\xC0";
	//engine->SetRip((PVOID)code);
	int counter = 0;
	while (engine->step())
	{
		counter++;
	}
	printf("Executed %d instructions\n", counter);

	return 0;
}

