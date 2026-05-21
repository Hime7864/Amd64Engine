#pragma once

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
					auto imm = *(UINT8*)(&RIP[2]);
					auto src = imm;
					auto dest = *(UINT8*)ptr;
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
					RIP += 1;
					status = true;
				}
			}break;
			case EMODE::REG_TO_REG:
			{
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
					auto imm = *(UINT32*)(&RIP[2]);
					if (Prefix.W)
					{
						auto src = imm;
						auto dest = *(UINT64*)ptr;
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
						RIP += 4;
						status = true;
					}
					else
					{
						if (Prefix.OperandSize)
						{
							auto src = imm & 0xFFFF;
							auto dest = *(UINT16*)ptr;
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
						else
						{
							auto src = imm;
							auto dest = *(UINT32*)ptr;
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
							RIP += 4;
							status = true;
						}
					}
				}
			}break;
			case EMODE::REG_TO_REG:
			{
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
						RIP += 4;
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
			}

			
		}break;
		};
	}break;
	};

	if(status)
		printf("Test");

	return status;
}

