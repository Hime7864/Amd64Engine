#pragma once

bool AssemblyState::service_and()
{
	bool status = false;

	switch (*RIP)
	{
	case 0x20:// r/m8	r8
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

			auto src = *(UINT8*)&GPR[modrm_register];
			auto dest = *(UINT8*)ptr;

			auto result = dest & src;
			*(UINT8*)ptr = result;

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
	case 0x21:// r/m16/32/64	r16/32/64
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			if (Prefix.W)
			{
				auto src = *(UINT64*)&GPR[modrm_register];
				auto dest = *(UINT64*)ptr;

				auto result = dest & src;

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
			else if (!Prefix.OperandSize)
			{
				auto src = *(UINT32*)&GPR[modrm_register];
				auto dest = *(UINT32*)ptr;

				auto result = dest & src;

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
			else
			{
				auto src = *(UINT16*)&GPR[modrm_register];
				auto dest = *(UINT16*)ptr;

				auto result = dest & src;

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
			status = true;
		}
	}break;
	case 0x22:// r8	r/m8
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

			auto src = *(UINT8*)ptr;
			auto dest = *(UINT8*)&GPR[modrm_register];
			auto result = dest & src;

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
	case 0x23:// r16/32/64	r/m16/32/64
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

			if (Prefix.W)
			{
				auto src = *(UINT64*)ptr;
				auto dest = GPR[modrm_register];

				auto result = dest & src;
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
			else if (!Prefix.OperandSize)
			{
				auto src = *(UINT32*)ptr;
				auto dest = *(UINT32*)&GPR[modrm_register];

				auto result = dest | src;
				GPR[modrm_register] = 0;
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
			else
			{
				auto src = *(UINT16*)ptr;
				auto dest = *(UINT16*)&GPR[modrm_register];

				auto result = dest | src;
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

			status = true;
		}
	}break;
	case 0x24:// AL	imm8
	{
		auto imm = *(UINT8*)(&RIP[1]);

		auto src = imm;
		auto dest = *(UINT8*)&GPR[(int)EGPR::RAX];

		auto result = dest & src;
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
	case 0x25://rAX	imm16/32
	{
		auto imm = *(UINT32*)(&RIP[1]);
		if (!Prefix.OperandSize)
		{
			auto src = imm;
			auto dest = *(UINT32*)&GPR[(int)EGPR::RAX];

			auto result = dest & src;
			GPR[(int)EGPR::RAX] = 0;
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
		else
		{
			auto src = imm & 0xFFFF;
			auto dest = *(UINT16*)&GPR[(int)EGPR::RAX];

			auto result = dest & src;
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
	}break;
	case 0x80:// r/m8	imm8
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto imm = *(UINT8*)(&RIP[0]);
			auto src = imm;
			auto dest = *(UINT8*)ptr;

			auto result = dest & src;
			*(UINT8*)ptr = result;

			FLAGS.SF = (result & 0x80) != 0;
			FLAGS.ZF = (result == 0);

			auto p = (UINT8)result;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.CF = 0;
			FLAGS.OF = 0;

			RIP += 1;
			status = true;
		}
	}break;
	case 0x81:// r/m16/32/64	imm16/32
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto imm = *(UINT32*)(&RIP[0]);
			if (Prefix.W)
			{
				auto src = (UINT64)imm;
				auto dest = *(UINT64*)ptr;

				auto result = dest & src;
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

				RIP += 4;
				status = true;
			}
			else if (!Prefix.OperandSize)
			{
				auto src = imm;
				auto dest = *(UINT32*)ptr;

				auto result = dest & src;
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

				RIP += 4;
				status = true; 
			}
			else
			{
				auto src = (UINT16)imm;
				auto dest = *(UINT16*)ptr;

				auto result = dest & src;
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

				RIP += 2;
				status = true;
			}
		}
	}break;
	case 0x83:// r/m16/32/64	imm8
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto imm_byte = *(INT8*)(&RIP[1]);

			if (Prefix.W)
			{
				auto src = (UINT64)(INT64)imm_byte;
				auto dest = *(UINT64*)ptr;

				auto result = dest & src;
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
			else if(!Prefix.OperandSize)
			{
				auto src = (UINT32)(INT32)imm_byte;
				auto dest = *(UINT32*)ptr;

				auto result = dest & src;
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
			else
			{
				auto src = (UINT16)(INT16)imm_byte;
				auto dest = *(UINT16*)ptr;

				auto result = dest & src;
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

			RIP++;
			status = true;
		}
	}break;
	};

	return status;
}

