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
	BYTE Register : 3;
	BYTE RegisterMemory : 3;
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
	bool service_mov();
	bool decode_mnemonic();
public:
	void SetRip(PVOID rip);
	void SetGPR(int index, UINT64 value);
	bool step();
};

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
	auto opcode = (BYTE*)RIP;
	switch (*opcode)
	{
	case 0x88:
	{
		auto modrm = (MODRM*)(opcode + 1);
		auto idx_reg = modrm->Register;
		auto idx_regmem = modrm->RegisterMemory;
		if (Prefix.R)
			idx_reg += 8;
		if (Prefix.B)
			idx_regmem += 8;

		*(BYTE*)&GPR[idx_regmem] = *(BYTE*)&GPR[idx_reg];

		RIP += 2;
		status = true;
	}break;
	case 0x89:
	{
		auto modrm = (MODRM*)(opcode + 1);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		{
			auto modrm = (MODRM*)(opcode + 1);
			if (modrm->Register == (BYTE)EGPR::RSP)
			{
				auto modrm2 = (MODRM*)(opcode + 2);
				auto mutiplier = 1ull << (BYTE)modrm2->Mode;

				if (modrm2->Register == (BYTE)EGPR::RBP)
				{
					auto idx_reg = modrm2->Register;
					auto idx_idx_regmem = modrm2->RegisterMemory;
					auto idx_regmem = modrm->RegisterMemory;
					if (Prefix.B)
						idx_reg += 8;
					if (Prefix.X)
						idx_idx_regmem += 8;
					if (Prefix.R)
						idx_regmem += 8;

					auto imm = *(INT32*)(opcode + 3);
					auto ptr = imm + GPR[idx_idx_regmem] * mutiplier;
					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					if (Prefix.W)
					{
						*(UINT64*)ptr = GPR[idx_regmem];
					}
					else
					{
						*(UINT32*)ptr = GPR[idx_regmem];
					}


					RIP += 7;
					status = true;
				}
				else
				{
					auto idx_reg = modrm2->Register;
					auto idx_idx_regmem = modrm2->RegisterMemory;
					auto idx_regmem = modrm->RegisterMemory;
					if (Prefix.B)
						idx_reg += 8;
					if (Prefix.X)
						idx_idx_regmem += 8;
					if (Prefix.R)
						idx_regmem += 8;


					auto ptr = GPR[idx_reg] + GPR[idx_idx_regmem] * mutiplier;
					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;


					if (Prefix.W)
					{
						*(UINT64*)ptr = GPR[idx_regmem];
					}
					else
					{
						*(UINT32*)ptr = GPR[idx_regmem];
					}
					RIP += 3;
					status = true;
				}
			}
			else
			{

				auto idx_reg = modrm->Register;
				auto idx_regmem = modrm->RegisterMemory;
				if (Prefix.B)
					idx_reg += 8;
				if (Prefix.R)
					idx_regmem += 8;

				auto ptr = GPR[idx_reg];
				if (Prefix.GS)
					ptr += GsBase;
				else if (Prefix.FS)
					ptr += FsBase;

				if (Prefix.W)
				{
					*(UINT64*)ptr = GPR[idx_regmem];
				}
				else
				{
					*(UINT32*)ptr = GPR[idx_regmem];
				}
				RIP += 2;
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto idx_reg = modrm->Register;
			auto idx_regmem = modrm->RegisterMemory;
			if(Prefix.R)
				idx_reg += 8;
			if (Prefix.B)
				idx_regmem += 8;

			if (Prefix.W)
			{
				GPR[idx_regmem] = GPR[idx_reg];
			}
			else
			{
				GPR[idx_regmem] = GPR[idx_reg] & 0xFFFFFFFFull;
			}
			RIP += 2;
			status = true;
		}break;
		};

	}break;
	case 0x8B:
	{
		auto modrm = (MODRM*)(opcode + 1);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		{
			auto modrm = (MODRM*)(opcode + 1);
			if (modrm->Register == (BYTE)EGPR::RSP)
			{
				auto modrm2 = (MODRM*)(opcode + 2);
				auto mutiplier = 1ull << (BYTE)modrm2->Mode;

				if (modrm2->Register == (BYTE)EGPR::RBP)
				{
					auto idx_reg = modrm2->Register;
					auto idx_idx_regmem = modrm2->RegisterMemory;
					auto idx_regmem = modrm->RegisterMemory;
					if (Prefix.B)
						idx_reg += 8;
					if (Prefix.X)
						idx_idx_regmem += 8;
					if (Prefix.R)
						idx_regmem += 8;

					auto imm = *(INT32*)(opcode + 3);
					auto ptr = imm + GPR[idx_idx_regmem] * mutiplier;
					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					if (Prefix.W)
					{
						GPR[idx_regmem] = *(UINT64*)ptr;
					}
					else
					{
						GPR[idx_regmem] = *(UINT32*)ptr;
					}

					RIP += 7;
					status = true;
				}
				else
				{
					auto idx_reg = modrm2->Register;
					auto idx_idx_regmem = modrm2->RegisterMemory;
					auto idx_regmem = modrm->RegisterMemory;
					if (Prefix.B)
						idx_reg += 8;
					if (Prefix.X)
						idx_idx_regmem += 8;
					if (Prefix.R)
						idx_regmem += 8;

					auto ptr = GPR[idx_reg] + GPR[idx_idx_regmem] * mutiplier;
					if (Prefix.GS)
						ptr += GsBase;
					else if (Prefix.FS)
						ptr += FsBase;

					if (Prefix.W)
					{
						GPR[idx_regmem] = *(UINT64*)ptr;
					}
					else
					{
						GPR[idx_regmem] = *(UINT32*)ptr;
					}
					RIP += 3;
					status = true;
				}
			}
			else
			{
				auto idx_reg = modrm->Register;
				auto idx_regmem = modrm->RegisterMemory;
				if (Prefix.B)
					idx_reg += 8;
				if (Prefix.R)
					idx_regmem += 8;

				auto ptr = GPR[idx_reg];
				if (Prefix.GS)
					ptr += GsBase;
				else if (Prefix.FS)
					ptr += FsBase;

				if (Prefix.W)
				{
					GPR[idx_regmem] = *(UINT64*)ptr;
				}
				else
				{
					GPR[idx_regmem] = *(UINT32*)ptr;
				}
				RIP += 2;
				status = true;
			}
		}break;
		};
	}break;
	case 0x8C:
	{
		auto modrm = (MODRM*)(opcode + 1);
		switch (modrm->RegisterMemory)
		{
		case 0:
			GPR[modrm->Register] = 0x2B;
			break;
		case 1:
			GPR[modrm->Register] = 0x33;
			break;
		case 2:
			GPR[modrm->Register] = 0x2B;
			break;
		case 3:
			GPR[modrm->Register] = 0x2B;
			break;
		case 4:
			GPR[modrm->Register] = 0x53;
			break;
		case 5:
			GPR[modrm->Register] = 0x2B;
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
		auto reg = *opcode & 0x7;
		if(Prefix.B)
			reg += 8;
		*(BYTE*)&GPR[reg] = *(BYTE*)(opcode + 1);
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
		auto reg = *opcode & 0x7;
		if (Prefix.B)
			reg += 8;

		if (Prefix.OperandSize)
		{
			*(WORD*)&GPR[reg] = *(WORD*)(opcode + 1);
			RIP += 3;
			status = true;
		}
		else
		{
			if (Prefix.W)
			{
				GPR[reg] = *(UINT64*)(opcode + 1);
				RIP += 9;
				status = true;
			}
			else
			{
				GPR[reg] = *(UINT32*)(opcode + 1);
				RIP += 5;
				status = true;
			}
		}
	}break;
	case 0xC7:
	{
		auto modrm = (MODRM*)(opcode + 1);
		auto imm = *(UINT32*)(opcode + 2);
		auto reg = modrm->Register;
		if (Prefix.B)
			reg += 8;

		GPR[reg] = (UINT64)imm;
		
		RIP += 6;
		status = true;

	}break;
	};

	return status;
}

bool AssemblyState::decode_mnemonic()
{
	bool status = false;

	// first past for prefixes

	Prefix = { 0 };

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

	printf("Decoding opcode: 0x%02X\n", *RIP);
	// third pass for primary opcode
	switch (*RIP)
	{
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
	//auto code = "\x89\xC0\xB8\xDE\x00\x00\x00\xB8\xAD\xDE\x00\x00\x8B\x00\x8B\x04\x00\x8B\x04\x40\x8B\x04\x80\x8B\x04\xC0\x8B\x04\xC5\xAD\xDE\x00\x00\x65\x8B\x04\x25\x60\x00\x00\x00\x65\x8B\x00\x65\x8B\x04\x00\x65\x8B\x04\xC0\x65\x48\x8B\x04\xC5\xAD\xDE\x00\x00\x48\x89\xC0\x48\xC7\xC0\xDE\x00\x00\x00\x48\xB8\xAD\xDE\xAD\xDE\x00\x00\x00\x00\x48\xB8\xAD\xDE\xAD\xDE\xAD\xDE\x00\x00\x48\x8B\x00\x48\x8B\x04\x00\x48\x8B\x04\xC0\x48\x8B\x04\xC5\xAD\xDE\x00\x00\x65\x48\x8B\x04\x25\x60\x00\x00\x00\x65\x48\x8B\x00\x65\x48\x8B\x04\x00\x65\x48\x8B\x04\x40\x65\x48\x8B\x04\x80\x65\x48\x8B\x04\xC0\x65\x48\x8B\x04\xC5\xAD\xDE\x00\x00\x89\x00\x89\x04\x00\x89\x04\xC0\x89\x04\xC5\xAD\xDE\x00\x00\x65\x89\x04\x25\x60\x00\x00\x00\x65\x89\x00\x65\x89\x04\x00\x65\x89\x04\xC0\x65\x48\x89\x04\xC5\xAD\xDE\x00\x00\x48\x89\x00\x48\x89\x04\x00\x48\x89\x04\xC0\x48\x89\x04\xC5\xAD\xDE\x00\x00\x65\x48\x89\x04\x25\x60\x00\x00\x00\x65\x48\x89\x00\x65\x48\x89\x04\x00\x65\x48\x89\x04\xC0\x65\x48\x89\x04\xC5\xAD\xDE\x00\x00";
	//auto code = "\xB0\xDE\x41\xB0\xDE\xB8\xDE\x00\x00\x00\x66\x41\xB8\xDE\x00\x49\xC7\xC0\xDE\x00\x00\x00\x48\xC7\xC0\xDE\x00\x00\x00\x48\xB8\xAD\xDE\xAD\xDE\x00\x00\x00\x00\x48\xB8\xAD\xDE\xAD\xDE\xAD\xDE\xAD\xDE";
	//engine->SetRip((PVOID)code);
	int counter = 0;
	while (engine->step())
	{
		counter++;
	}
	printf("Executed %d instructions\n", counter);

	return 0;
}

