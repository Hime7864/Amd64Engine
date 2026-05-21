#pragma once

bool AssemblyState::service_rcl()
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
				auto src = RIP[2];
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			auto dest = *(UINT8*)&GPR[modrm_register_memory];
			auto src = RIP[2];

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
					auto src = RIP[2];
				}
				else
				{
					if (Prefix.OperandSize)
					{
						auto dest = *(UINT16*)ptr;
						auto src = RIP[2];
					}
					else
					{
						auto dest = *(UINT32*)ptr;
						auto src = RIP[2];
					}
				}
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto dest = *(UINT64*)&GPR[modrm_register_memory];
				auto src = RIP[2];
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto dest = *(UINT16*)&GPR[modrm_register_memory];
					auto src = RIP[2];
				}
				else
				{
					auto dest = *(UINT32*)&GPR[modrm_register_memory];
					auto src = RIP[2];
				}
			}
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
				auto src = 1;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			auto dest = *(UINT8*)&GPR[modrm_register_memory];
			auto src = 1;
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
					auto src = 1;
				}
				else
				{
					if (Prefix.OperandSize)
					{
						auto dest = *(UINT16*)ptr;
						auto src = 1;
					}
					else
					{
						auto dest = *(UINT32*)ptr;
						auto src = 1;
					}
				}
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto dest = *(UINT64*)&GPR[modrm_register_memory];
				auto src = 1;
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto dest = *(UINT16*)&GPR[modrm_register_memory];
					auto src = 1;
				}
				else
				{
					auto dest = *(UINT32*)&GPR[modrm_register_memory];
					auto src = 1;
				}
			}
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
				auto src = *(UINT8*)&GPR[(int)EGPR::RCX];
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			auto dest = *(UINT8*)&GPR[modrm_register_memory];
			auto src = *(UINT8*)&GPR[(int)EGPR::RCX];
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
					auto src = (UINT64) * (UINT8*)&GPR[(int)EGPR::RCX];
				}
				else
				{
					if (Prefix.OperandSize)
					{
						auto dest = *(UINT16*)ptr;
						auto src = (UINT16) * (UINT8*)&GPR[(int)EGPR::RCX];
					}
					else
					{
						auto dest = *(UINT32*)ptr;
						auto src = (UINT32) * (UINT8*)&GPR[(int)EGPR::RCX];
					}
				}
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				auto dest = *(UINT64*)&GPR[modrm_register_memory];
				auto src = (UINT64) * (UINT8*)&GPR[(int)EGPR::RCX];
			}
			else
			{
				if (Prefix.OperandSize)
				{
					auto dest = *(UINT16*)&GPR[modrm_register_memory];
					auto src = (UINT16) * (UINT8*)&GPR[(int)EGPR::RCX];
				}
				else
				{
					auto dest = *(UINT32*)&GPR[modrm_register_memory];
					auto src = (UINT32) * (UINT8*)&GPR[(int)EGPR::RCX];
				}
			}
		}break;
		};
	}break;
	}

	if(status)
		printf("Rotate Carry Left");

	return status;
}

