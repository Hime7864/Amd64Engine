#pragma once

bool AssemblyState::service_ror()
{
	bool status = false;

	switch (*RIP)
	{
	case 0xC0:
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
				auto dest = *(UINT8*)ptr;
				UINT8 count = RIP[2];
				UINT8 masked = count & 7;

				if (masked)
				{
					auto result = (dest >> masked) | (dest << (8 - masked));
					*(UINT8*)ptr = result;

					FLAGS.CF = (result >> 7) & 1;
					if (masked == 1)
						FLAGS.OF = ((result >> 7) & 1) ^ ((result >> 6) & 1);
				}
				RIP++;
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			auto dest = *(UINT8*)&GPR[modrm_register_memory];
			UINT8 count = RIP[2];
			UINT8 masked = count & 7;

			if (masked)
			{
				auto result = (dest >> masked) | (dest << (8 - masked));
				*(UINT8*)&GPR[modrm_register_memory] = result;

				FLAGS.CF = (result >> 7) & 1;
				if (masked == 1)
					FLAGS.OF = ((result >> 7) & 1) ^ ((result >> 6) & 1);
			}
			RIP += 3;
			status = true;
		}break;
		};
	}break;
	case 0xC1:
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
				if (Prefix.W)
				{
					auto dest = *(UINT64*)ptr;
					UINT8 count = RIP[2];
					UINT8 masked = count & 63;

					if (masked)
					{
						auto result = (dest >> masked) | (dest << (64 - masked));
						*(UINT64*)ptr = result;

						FLAGS.CF = (result >> 63) & 1;
						if (masked == 1)
							FLAGS.OF = ((result >> 63) & 1) ^ ((result >> 62) & 1);
					}
				}
				else
				{
					if (Prefix.OperandSize)
					{
						auto dest = *(UINT16*)ptr;
						UINT8 count = RIP[2];
						UINT8 masked = count & 15;

						if (masked)
						{
							auto result = (dest >> masked) | (dest << (16 - masked));
							*(UINT16*)ptr = result;

							FLAGS.CF = (result >> 15) & 1;
							if (masked == 1)
								FLAGS.OF = ((result >> 15) & 1) ^ ((result >> 14) & 1);
						}
					}
					else
					{
						auto dest = *(UINT32*)ptr;
						UINT8 count = RIP[2];
						UINT8 masked = count & 31;

						if (masked)
						{
							auto result = (dest >> masked) | (dest << (32 - masked));
							*(UINT32*)ptr = result;

							FLAGS.CF = (result >> 31) & 1;
							if (masked == 1)
								FLAGS.OF = ((result >> 31) & 1) ^ ((result >> 30) & 1);
						}
					}
				}
				RIP++;
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto dest = *(UINT64*)&GPR[modrm_register_memory];
				UINT8 count = RIP[2];
				UINT8 masked = count & 63;

				if (masked)
				{
					auto result = (dest >> masked) | (dest << (64 - masked));
					*(UINT64*)&GPR[modrm_register_memory] = result;

					FLAGS.CF = (result >> 63) & 1;
					if (masked == 1)
						FLAGS.OF = ((result >> 63) & 1) ^ ((result >> 62) & 1);
				}
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto dest = *(UINT16*)&GPR[modrm_register_memory];
					UINT8 count = RIP[2];
					UINT8 masked = count & 15;

					if (masked)
					{
						auto result = (dest >> masked) | (dest << (16 - masked));
						*(UINT16*)&GPR[modrm_register_memory] = result;

						FLAGS.CF = (result >> 15) & 1;
						if (masked == 1)
							FLAGS.OF = ((result >> 15) & 1) ^ ((result >> 14) & 1);
					}
				}
				else
				{
					auto dest = *(UINT32*)&GPR[modrm_register_memory];
					UINT8 count = RIP[2];
					UINT8 masked = count & 31;

					if (masked)
					{
						auto result = (dest >> masked) | (dest << (32 - masked));
						GPR[modrm_register_memory] = 0;
						*(UINT32*)&GPR[modrm_register_memory] = result;

						FLAGS.CF = (result >> 31) & 1;
						if (masked == 1)
							FLAGS.OF = ((result >> 31) & 1) ^ ((result >> 30) & 1);
					}
				}
			}
			RIP += 3;
			status = true;

		}break;
		};
	}break;
	case 0xD0:
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
				auto dest = *(UINT8*)ptr;
				UINT8 count = 1;
				UINT8 masked = count & 7;

				if (masked)
				{
					auto result = (dest >> masked) | (dest << (8 - masked));
					*(UINT8*)ptr = result;

					FLAGS.CF = (result >> 7) & 1;
					if (masked == 1)
						FLAGS.OF = ((result >> 7) & 1) ^ ((result >> 6) & 1);
				}
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			auto dest = *(UINT8*)&GPR[modrm_register_memory];
			UINT8 count = 1;
			UINT8 masked = count & 7;

			if (masked)
			{
				auto result = (dest >> masked) | (dest << (8 - masked));
				*(UINT8*)&GPR[modrm_register_memory] = result;

				FLAGS.CF = (result >> 7) & 1;
				if (masked == 1)
					FLAGS.OF = ((result >> 7) & 1) ^ ((result >> 6) & 1);
			}
			RIP++;
			status = true;
		}break;
		};
	}break;
	case 0xD1:
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
				if (Prefix.W)
				{
					auto dest = *(UINT64*)ptr;
					UINT8 count = 1;
					UINT8 masked = count & 63;

					if (masked)
					{
						auto result = (dest >> masked) | (dest << (64 - masked));
						*(UINT64*)ptr = result;
						FLAGS.CF = (result >> 63) & 1;
						if (masked == 1)
							FLAGS.OF = ((result >> 63) & 1) ^ ((result >> 62) & 1);
					}
				}
				else
				{
					if (Prefix.OperandSize)
					{
						auto dest = *(UINT16*)ptr;
						UINT8 count = 1;
						UINT8 masked = count & 15;

						if (masked)
						{
							auto result = (dest >> masked) | (dest << (16 - masked));
							*(UINT16*)ptr = result;
							FLAGS.CF = (result >> 15) & 1;
							if (masked == 1)
								FLAGS.OF = ((result >> 15) & 1) ^ ((result >> 14) & 1);
						}
					}
					else
					{
						auto dest = *(UINT32*)ptr;
						UINT8 count = 1;
						UINT8 masked = count & 31;

						if (masked)
						{
							auto result = (dest >> masked) | (dest << (32 - masked));
							*(UINT32*)ptr = result;
							FLAGS.CF = (result >> 31) & 1;
							if (masked == 1)
								FLAGS.OF = ((result >> 31) & 1) ^ ((result >> 30) & 1);
						}
					}
				}
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto dest = *(UINT64*)&GPR[modrm_register_memory];
				UINT8 count = 1;
				UINT8 masked = count & 63;

				if (masked)
				{
					auto result = (dest >> masked) | (dest << (64 - masked));
					*(UINT64*)&GPR[modrm_register_memory] = result;
					FLAGS.CF = (result >> 63) & 1;
					if (masked == 1)
						FLAGS.OF = ((result >> 63) & 1) ^ ((result >> 62) & 1);
				}
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto dest = *(UINT16*)&GPR[modrm_register_memory];
					UINT8 count = 1;
					UINT8 masked = count & 15;

					if (masked)
					{
						auto result = (dest >> masked) | (dest << (16 - masked));
						*(UINT16*)&GPR[modrm_register_memory] = result;
						FLAGS.CF = (result >> 15) & 1;
						if (masked == 1)
							FLAGS.OF = ((result >> 15) & 1) ^ ((result >> 14) & 1);
					}
				}
				else
				{
					auto dest = *(UINT32*)&GPR[modrm_register_memory];
					UINT8 count = 1;
					UINT8 masked = count & 31;

					if (masked)
					{
						auto result = (dest >> masked) | (dest << (32 - masked));
						GPR[modrm_register_memory] = 0;
						*(UINT32*)&GPR[modrm_register_memory] = result;
						FLAGS.CF = (result >> 31) & 1;
						if (masked == 1)
							FLAGS.OF = ((result >> 31) & 1) ^ ((result >> 30) & 1);
					}
				}
			}
			RIP++;
			status = true;
		}break;
		};
	}break;
	case 0xD2:
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
				auto dest = *(UINT8*)ptr;
				UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
				UINT8 masked = count & 7;

				if (masked)
				{
					auto result = (dest >> masked) | (dest << (8 - masked));
					*(UINT8*)ptr = result;
					FLAGS.CF = (result >> 7) & 1;
					if (masked == 1)
						FLAGS.OF = ((result >> 7) & 1) ^ ((result >> 6) & 1);
				}
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			auto dest = *(UINT8*)&GPR[modrm_register_memory];
			UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
			UINT8 masked = count & 7;

			if (masked)
			{
				auto result = (dest >> masked) | (dest << (8 - masked));
				*(UINT8*)&GPR[modrm_register_memory] = result;
				FLAGS.CF = (result >> 7) & 1;
				if (masked == 1)
					FLAGS.OF = ((result >> 7) & 1) ^ ((result >> 6) & 1);
			}
			status = true;
			RIP++;
		}break;
		};
	}break;
	case 0xD3:
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
				if (Prefix.W)
				{
					auto dest = *(UINT64*)ptr;
					UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
					UINT8 masked = count & 63;

					if (masked)
					{
						auto result = (dest >> masked) | (dest << (64 - masked));
						*(UINT64*)ptr = result;
						FLAGS.CF = (result >> 63) & 1;
						if (masked == 1)
							FLAGS.OF = ((result >> 63) & 1) ^ ((result >> 62) & 1);
					}
				}
				else
				{
					if (Prefix.OperandSize)
					{
						auto dest = *(UINT16*)ptr;
						UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
						UINT8 masked = count & 15;

						if (masked)
						{
							auto result = (dest >> masked) | (dest << (16 - masked));
							*(UINT16*)ptr = result;
							FLAGS.CF = (result >> 15) & 1;
							if (masked == 1)
								FLAGS.OF = ((result >> 15) & 1) ^ ((result >> 14) & 1);
						}
					}
					else
					{
						auto dest = *(UINT32*)ptr;
						UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
						UINT8 masked = count & 31;

						if (masked)
						{
							auto result = (dest >> masked) | (dest << (32 - masked));
							*(UINT32*)ptr = result;
							FLAGS.CF = (result >> 31) & 1;
							if (masked == 1)
								FLAGS.OF = ((result >> 31) & 1) ^ ((result >> 30) & 1);
						}
					}
				}
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto dest = *(UINT64*)&GPR[modrm_register_memory];
				UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
				UINT8 masked = count & 63;

				if (masked)
				{
					auto result = (dest >> masked) | (dest << (64 - masked));
					*(UINT64*)&GPR[modrm_register_memory] = result;
					FLAGS.CF = (result >> 63) & 1;
					if (masked == 1)
						FLAGS.OF = ((result >> 63) & 1) ^ ((result >> 62) & 1);
				}
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto dest = *(UINT16*)&GPR[modrm_register_memory];
					UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
					UINT8 masked = count & 15;

					if (masked)
					{
						auto result = (dest >> masked) | (dest << (16 - masked));
						*(UINT16*)&GPR[modrm_register_memory] = result;
						FLAGS.CF = (result >> 15) & 1;
						if (masked == 1)
							FLAGS.OF = ((result >> 15) & 1) ^ ((result >> 14) & 1);
					}
				}
				else
				{
					auto dest = *(UINT32*)&GPR[modrm_register_memory];
					UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
					UINT8 masked = count & 31;

					if (masked)
					{
						auto result = (dest >> masked) | (dest << (32 - masked));
						GPR[modrm_register_memory] = 0;
						*(UINT32*)&GPR[modrm_register_memory] = result;
						FLAGS.CF = (result >> 31) & 1;
						if (masked == 1)
							FLAGS.OF = ((result >> 31) & 1) ^ ((result >> 30) & 1);
					}
				}
			}

			status = true;
			RIP++;
		}break;
		};
	}break;
	}

	if (status)
		printf("ROR");

	return status;
}

