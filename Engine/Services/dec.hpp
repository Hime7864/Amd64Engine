#pragma once

bool AssemblyState::service_dec()
{
	bool status = false;

	switch (*RIP)
	{
	case 0xFE:
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
				auto value = *(UINT8*)ptr;
				value--;
				*(UINT8*)ptr = value;

				FLAGS.SF = (value & 0x80) != 0;
				FLAGS.ZF = (value == 0);

				auto p = (UINT8)value;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.OF = (value == 0x7F);
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
			auto value = *(BYTE*)&GPR[modrm_register_memory];
			value++;
			*(BYTE*)&GPR[modrm_register_memory] = value;

			FLAGS.SF = (value & 0x80) != 0;
			FLAGS.ZF = (value == 0);

			auto p = (UINT8)value;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.OF = (value == 0x7F);

			RIP += 2;
			status = true;
		}break;
		}
	}break;
	case 0xFF:
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
					auto value = *(UINT64*)ptr;
					value--;
					*(UINT64*)ptr = value;

					FLAGS.SF = (value & 0x8000000000000000) != 0;
					FLAGS.ZF = (value == 0);

					auto p = (UINT8)value;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.OF = (value == 0x7FFFFFFFFFFFFFFF);
				}
				else
				{
					if (Prefix.OperandSize)
					{
						auto value = *(UINT16*)ptr;
						value--;
						*(UINT16*)ptr = value;

						FLAGS.SF = (value & 0x8000) != 0;
						FLAGS.ZF = (value == 0);

						auto p = (UINT8)value;
						p ^= p >> 4;
						p ^= p >> 2;
						p ^= p >> 1;

						FLAGS.PF = (p & 1) == 0;
						FLAGS.OF = (value == 0x7FFF);
					}
					else
					{
						auto value = *(UINT32*)ptr;
						value--;
						*(UINT32*)ptr = value;

						FLAGS.SF = (value & 0x80000000) != 0;
						FLAGS.ZF = (value == 0);

						auto p = (UINT8)value;
						p ^= p >> 4;
						p ^= p >> 2;
						p ^= p >> 1;

						FLAGS.PF = (p & 1) == 0;
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
				auto value = *(UINT64*)&GPR[modrm_register_memory];
				value--;
				*(UINT64*)&GPR[modrm_register_memory] = value;

				FLAGS.SF = (value & 0x8000000000000000) != 0;
				FLAGS.ZF = (value == 0);

				auto p = (UINT8)value;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.OF = (value == 0x7FFFFFFFFFFFFFFF);
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto value = *(UINT16*)&GPR[modrm_register_memory];
					value--;
					*(UINT16*)&GPR[modrm_register_memory] = value;

					FLAGS.SF = (value & 0x8000) != 0;
					FLAGS.ZF = (value == 0);

					auto p = (UINT8)value;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.OF = (value == 0x7FFF);
				}
				else
				{
					auto value = *(UINT32*)&GPR[modrm_register_memory];
					value--;
					*(UINT32*)&GPR[modrm_register_memory] = value;

					FLAGS.SF = (value & 0x80000000) != 0;
					FLAGS.ZF = (value == 0);

					auto p = (UINT8)value;
					p ^= p >> 4;
					p ^= p >> 2;
					p ^= p >> 1;

					FLAGS.PF = (p & 1) == 0;
					FLAGS.OF = (value == 0x7FFFFFFF);
				}
			}
			RIP += 2;
			status = true;
		}break;
		}
	}break;
	};

	if (status)
		printf("Decrease");

	return status;
}

