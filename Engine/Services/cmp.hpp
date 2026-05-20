#pragma once

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
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

				auto src = *(UINT8*)&GPR[modrm_register];
				auto dest = *(UINT8*)ptr;

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


			auto src = *(UINT8*)&GPR[modrm_register];
			auto dest = *(UINT8*)&GPR[modrm_register_memory];

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

					auto src = imm;
					auto dest = *(UINT8*)ptr;

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

					RIP += 1;
					status = true;
				}
			}break;
			case EMODE::REG_TO_REG:
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

				auto src = imm;
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
				RIP += 3;
				status = true;
			}break;
			};
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
			switch (modrm->Mode)
			{
			case EMODE::MEM_0_BIT_DISP:
			case EMODE::MEM_8_BIT_DISP:
			case EMODE::MEM_32_BIT_DISP:
			{
				auto ptr = GetDisplacementPtr();
				if (ptr)
				{
					if (Prefix.W)
					{
						auto src = (UINT64)(INT64)(INT32)imm;
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

						RIP += 4;
						status = true;
					}
					else
					{
						if (Prefix.OperandSize)
						{
							auto src = (UINT16)(INT16)(INT32)imm;
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

							RIP += 2;
							status = true;
						}
						else
						{
							auto src = (UINT32)(INT32)imm;
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

							RIP += 4;
							status = true;
						}
					}
				}
			}break;
			case EMODE::REG_TO_REG:
			{
				if (Prefix.W)
				{
					auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
					auto src = (UINT64)(INT64)(INT32)imm;
					auto dest = *(UINT64*)&GPR[modrm_register_memory];

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

					RIP += 6;
					status = true;
				}
				else
				{
					if (Prefix.OperandSize)
					{
						auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
						auto src = (UINT16)(INT16)(INT32)imm;
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

						RIP += 4;
						status = true;
					}
					else
					{
						auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
						auto src = (UINT32)(INT32)imm;
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

						RIP += 6;
						status = true;
					}
				}
			}break;
			};
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
			switch (modrm->Mode)
			{
			case EMODE::MEM_0_BIT_DISP:
			case EMODE::MEM_8_BIT_DISP:
			case EMODE::MEM_32_BIT_DISP:
			{
				auto ptr = GetDisplacementPtr();
				if (ptr)
				{
					auto imm_byte = *(INT8*)RIP;

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
					else
					{
						if (Prefix.OperandSize)
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
					}

					RIP++;
					status = true;
				}
			}break;
			case EMODE::REG_TO_REG:
			{
				auto imm_byte = *(INT8*)(&RIP[2]);
				auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

				if (Prefix.W)
				{
					auto src = (UINT64)(INT64)imm_byte;
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
				else if (Prefix.OperandSize)
				{
					auto src = (UINT16)(INT16)imm_byte;
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
					auto src = (UINT32)(INT32)imm_byte;
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

				RIP += 3;
				status = true;
			}break;
			};
		}break;
		};
	}break;
	}

	if (status)
		printf("CMP");

	return status;
}

