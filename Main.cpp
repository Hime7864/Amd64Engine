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
	RIP = 5
};

enum struct EMOD : BYTE
{
	MEM_0_BIT_DISP = 0,
	MEM_8_BIT_DISP = 1,
	MEM_32_BIT_DISP = 2,
	REG_TO_REG = 3
};

struct MOD_RM
{
	BYTE Reg : 3;
	BYTE R_M : 3;
	EMOD Mod : 2;
};

enum struct EPREFIXCODE : BYTE
{
	REX = 4
};

enum struct EMNEMONICTYPE : BYTE
{
	Standard,
	Inverted,
	Immediate32,
	Immediate64
};

struct MNEMONIC_PREFIX
{
	BYTE B : 1;
	BYTE X : 1;
	BYTE R : 1;
	BYTE W : 1;
	EPREFIXCODE code : 4;
};

class AssemblyState
{
private:
	UINT64 GPR[16];
	EFLAGS Flags;
	MNEMONIC_PREFIX MnemonicPrefix;
	EMNEMONICTYPE MnemonicType;
	bool service_push();
	bool service_mov();
	bool service_lea();
	bool service_jmp();

	bool service_mnemonic();
public:
	void SetRip(PVOID rip);
	void SetGPR(int index, UINT64 value);
	bool step();
};

void AssemblyState::SetRip(PVOID rip)
{
	GPR[(int)EGPR::RIP] = (UINT64)rip;
}

void AssemblyState::SetGPR(int index, UINT64 value)
{
	if (index < 16)
		GPR[index] = value;
}

bool AssemblyState::service_push()
{
	bool status = false;
	auto mod0 = *(MOD_RM*)GPR[(int)EGPR::RIP];
	GPR[(int)EGPR::RIP]++;
	if ((int)mod0.Mod == 1)
	{
		*(UINT64*)GPR[(int)EGPR::RSP] = GPR[mod0.Reg];
		status = true;
	}
	else
	{
		if (mod0.R_M == 6)
		{
			if (mod0.Reg == (BYTE)EGPR::RSP)
			{
				auto mod1 = *(MOD_RM*)GPR[(int)EGPR::RIP];
				GPR[(int)EGPR::RIP]++;
				if (mod1.Mod == (EMOD)0)
				{
					*(UINT64*)GPR[(int)EGPR::RSP] = *(UINT64*)GPR[mod1.Reg];
					status = true;
				}
				else
				{
					if (mod1.Reg == (BYTE)EGPR::RIP)
					{
						*(UINT64*)GPR[(int)EGPR::RSP] = (GPR[mod1.R_M] * *(UINT64*)(1lu << (int)mod1.Mod) + *(UINT32*)(GPR[(int)EGPR::RIP]));
						GPR[(int)EGPR::RIP] += 4;
						status = true;
					}
					else
					{
						*(UINT64*)GPR[(int)EGPR::RSP] = *(UINT64*)(GPR[mod1.Reg] + GPR[mod1.R_M] * (1lu << (int)mod1.Mod));
						status = true;
					}
				}
			}
			else
			{
				*(UINT64*)GPR[(int)EGPR::RSP] = *(UINT64*)GPR[mod0.Reg];
				status = true;
			}
		}
	}
	GPR[(int)EGPR::RSP] -= 8;
	return status;
}

bool AssemblyState::service_mov()
{
	bool status = false;
	switch (MnemonicType)
	{
	case EMNEMONICTYPE::Standard:
	{
		auto mod0 = *(MOD_RM*)GPR[(int)EGPR::RIP];
		GPR[(int)EGPR::RIP]++;
		switch (mod0.Mod)
		{
		case EMOD::MEM_0_BIT_DISP:
		{
			if (mod0.Reg == 4)
			{
				auto mod1 = *(MOD_RM*)(GPR[(int)EGPR::RIP]);
				GPR[(int)EGPR::RIP]++;
				if (mod1.Reg == (BYTE)EGPR::RIP)
				{
					GPR[mod0.R_M] = *(UINT64*)(GPR[mod1.R_M] * (1 << (int)mod1.Mod) + *(UINT32*)(GPR[(int)EGPR::RIP]));
					GPR[(int)EGPR::RIP] += 4;
					status = true;
				}
				else
				{
					GPR[mod0.R_M] = *(UINT64*)(GPR[mod1.Reg] + GPR[mod1.R_M] * (1lu << (int)mod1.Mod));
					status = true;
				}
			}
			else if (mod0.Reg == (BYTE)EGPR::RIP)
			{
				GPR[mod0.R_M] = *(UINT64*)(GPR[(int)EGPR::RIP] + *(UINT32*)(GPR[(int)EGPR::RIP]) + 4);
				GPR[(int)EGPR::RIP] += 4;
				status = true;
			}
			else
			{
				GPR[mod0.Reg] = *(UINT64*)GPR[mod0.R_M];
				GPR[(int)EGPR::RIP] += 1;
				status = true;
			}
		}break;
		case EMOD::MEM_8_BIT_DISP:
		{
			GPR[(int)EGPR::RIP]++;
			*(UINT64*)(GPR[mod0.Reg] + *(BYTE*)GPR[(int)EGPR::RIP]) = GPR[mod0.R_M];
			GPR[(int)EGPR::RIP]++;
			status = true;
		}break;
		case EMOD::MEM_32_BIT_DISP:
		{
			GPR[(int)EGPR::RIP]++;
			*(UINT64*)(GPR[mod0.Reg] + *(UINT32*)GPR[(int)EGPR::RIP]) = GPR[mod0.R_M];
			GPR[(int)EGPR::RIP] += 4;
			status = true;
		}break;
		case EMOD::REG_TO_REG:
		{
			GPR[mod0.Reg] = GPR[mod0.R_M];
			GPR[(int)EGPR::RIP]++;
			status = true;
		}break;
		default:
			break;
		}
	}break;
	case EMNEMONICTYPE::Inverted:
	{
		auto mod0 = *(MOD_RM*)GPR[(int)EGPR::RIP];
		GPR[(int)EGPR::RIP]++;
		switch (mod0.Mod)
		{
		case EMOD::MEM_0_BIT_DISP:
		{
			if (mod0.Reg == 4)
			{
				auto mod1 = *(MOD_RM*)(GPR[(int)EGPR::RIP]);
				GPR[(int)EGPR::RIP]++;
				if (mod1.Reg == (BYTE)EGPR::RIP)
				{
					GPR[mod0.R_M] = *(UINT64*)(GPR[mod1.R_M] * (1 << (int)mod1.Mod) + *(UINT32*)(GPR[(int)EGPR::RIP]));
					GPR[(int)EGPR::RIP] += 4;
					status = true;
				}
				else
				{
					GPR[mod0.R_M] = *(UINT64*)(GPR[mod1.Reg] + GPR[mod1.R_M] * (1lu << (int)mod1.Mod));
					status = true;
				}
			}
			else if (mod0.Reg == (BYTE)EGPR::RIP)
			{
				GPR[mod0.R_M] = *(UINT64*)(GPR[(int)EGPR::RIP] + *(UINT32*)(GPR[(int)EGPR::RIP]) + 4);
				GPR[(int)EGPR::RIP] += 4;
				status = true;
			}
			else
			{
				GPR[mod0.R_M] = *(UINT64*)GPR[mod0.Reg];
				GPR[(int)EGPR::RIP] += 1;
				status = true;
			}
		}break;
		case EMOD::MEM_8_BIT_DISP:
		{
			GPR[mod0.R_M] = *(UINT64*)(GPR[mod0.Reg] + *(BYTE*)GPR[(int)EGPR::RIP]);
			GPR[(int)EGPR::RIP] += 1;
			status = true;
		}break;
		case EMOD::MEM_32_BIT_DISP:
		{
			GPR[mod0.R_M] = *(UINT64*)(GPR[mod0.Reg] + *(UINT32*)GPR[(int)EGPR::RIP]);
			GPR[(int)EGPR::RIP] += 4;
			status = true;
		}break;
		case EMOD::REG_TO_REG:
		{
			GPR[mod0.R_M] = GPR[mod0.Reg];
			GPR[(int)EGPR::RIP] += 1;
			status = true;
		}break;
		default:
			break;
		}
	}break;
	case EMNEMONICTYPE::Immediate32:
	{
		auto mod0 = *(MOD_RM*)GPR[(int)EGPR::RIP];
		GPR[(int)EGPR::RIP]++;
		GPR[mod0.Reg] = *(UINT32*)GPR[(int)EGPR::RIP];
		GPR[(int)EGPR::RIP] += 4;
		status = true;
	}break;
	case EMNEMONICTYPE::Immediate64:
	{
		auto mod0 = *(MOD_RM*)GPR[(int)EGPR::RIP];
		GPR[(int)EGPR::RIP]++;
		GPR[mod0.Reg] = *(UINT64*)GPR[(int)EGPR::RIP];
		GPR[(int)EGPR::RIP] += 8;
		status = true;
	}break;
	default:
		break;
	}
	return status;
}

bool AssemblyState::service_lea()
{
	bool status = false;
	switch (MnemonicType)
	{
	case EMNEMONICTYPE::Inverted:
	{
		auto mod0 = *(MOD_RM*)GPR[(int)EGPR::RIP];
		GPR[(int)EGPR::RIP]++;
		switch (mod0.Mod)
		{
		case EMOD::MEM_0_BIT_DISP:
		{
			if (mod0.Reg == 4)
			{
				auto mod1 = *(MOD_RM*)(GPR[(int)EGPR::RIP]);
				GPR[(int)EGPR::RIP]++;
				if (mod1.Reg == (BYTE)EGPR::RIP)
				{
					GPR[mod0.R_M] = GPR[mod1.R_M] * (1 << (int)mod1.Mod) + *(UINT32*)(GPR[(int)EGPR::RIP]);
					GPR[(int)EGPR::RIP] += 4;
					status = true;
				}
				else
				{
					GPR[mod0.R_M] = GPR[mod1.Reg] + GPR[mod1.R_M] * (1lu << (int)mod1.Mod);
					status = true;
				}
			}
			else if (mod0.Reg == (BYTE)EGPR::RIP)
			{
				GPR[mod0.R_M] = GPR[(int)EGPR::RIP] + *(UINT32*)(GPR[(int)EGPR::RIP]) + 4;
				GPR[(int)EGPR::RIP] += 4;
				status = true;
			}
			else
			{
				GPR[mod0.R_M] = GPR[mod0.Reg];
				GPR[(int)EGPR::RIP] += 1;
				status = true;
			}
		}break;
		case EMOD::MEM_8_BIT_DISP:
		{
			GPR[mod0.R_M] = (GPR[mod0.Reg] + *(BYTE*)GPR[(int)EGPR::RIP]);
			GPR[(int)EGPR::RIP] += 1;
			status = true;
		}break;
		case EMOD::MEM_32_BIT_DISP:
		{
			GPR[mod0.R_M] = (GPR[mod0.Reg] + *(UINT32*)GPR[(int)EGPR::RIP]);
			GPR[(int)EGPR::RIP] += 4;
			status = true;
		}break;
		default:
			break;
		}
	}break;
	default:
		break;
	}
	return status;
}

bool AssemblyState::service_jmp()
{
	GPR[(int)EGPR::RIP] = (INT64)GPR[(int)EGPR::RIP] + *(int*)GPR[(int)EGPR::RIP] + 4;
	return true;
}

bool AssemblyState::service_mnemonic()
{
	bool status = false;
	switch (*(BYTE*)GPR[(int)EGPR::RIP])
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
		MnemonicType = EMNEMONICTYPE::Standard;
		status = service_push();
	}break;
	case 0x89:
	{
		MnemonicType = EMNEMONICTYPE::Standard;
		GPR[(int)EGPR::RIP]++;
		status = service_mov();
	}break;
	case 0x8B:
	{
		MnemonicType = EMNEMONICTYPE::Inverted;
		GPR[(int)EGPR::RIP]++;
		status = service_mov();
	}break;
	case 0x8D:
	{
		MnemonicType = EMNEMONICTYPE::Inverted;
		GPR[(int)EGPR::RIP]++;
		status = service_lea();
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
		MnemonicType = EMNEMONICTYPE::Immediate64;
		status = service_mov();
	}break;
	case 0xC7:
	{
		MnemonicType = EMNEMONICTYPE::Immediate32;
		GPR[(int)EGPR::RIP]++;
		status = service_mov();
	}break;
	case 0xE9:
	{
		GPR[(int)EGPR::RIP]++;
		status = service_jmp();
	}break;
	case 0xFF:
	{
		GPR[(int)EGPR::RIP]++;
		switch (*(BYTE*)GPR[(int)EGPR::RIP])
		{
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
			status = service_push();
			break;
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
			status = service_push();
			break;
		default:
			break;
		}
	}break;
	default:
		break;
	}
	return status;
}

bool AssemblyState::step()
{
	bool status = false;

	auto last_rip = GPR[(int)EGPR::RIP];

	for (int i = 0; i < 15; i++)
		printf("%02X ", ((BYTE*)GPR[(int)EGPR::RIP])[i]);
	printf("\n");

	MnemonicPrefix = *(MNEMONIC_PREFIX*)GPR[(int)EGPR::RIP];
	if (MnemonicPrefix.code == EPREFIXCODE::REX)
		GPR[(int)EGPR::RIP]++;
	else
		MnemonicPrefix = { 0 };

	status = service_mnemonic();

	if(!status)
	{
		printf("[FAILED] ");
		for (int i = 0; i < 15; i++)
			printf("%02X ", ((BYTE*)last_rip)[i]);
		printf("\n");
	}


	auto byte_code = (BYTE*)GPR[(int)EGPR::RIP];
	auto code = (byte_code[0] & 0xF0) >> 4;
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
	//auto code = "\xFF\x34\xD8\xFF\x34\xDD\xAD\xDE\x00\x00";
	//engine->SetRip((PVOID)code);
	//engine->SetGPR((int)EGPR::RAX, (UINT64)malloc(0x1000));
	//engine->SetGPR((int)EGPR::RBX, 1);

	while (engine->step())
	{

	}

	return 0;
}