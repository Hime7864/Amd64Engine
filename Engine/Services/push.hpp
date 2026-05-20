#pragma once

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
		
		if (Prefix.OperandSize)
		{
			GPR[(int)EGPR::RSP] -= 2;
			*(UINT16*)GPR[(int)EGPR::RSP] = (UINT16)GPR[reg];
		}
		else
		{
			GPR[(int)EGPR::RSP] -= 8;
			*(UINT64*)GPR[(int)EGPR::RSP] = GPR[reg];
		}
		
		RIP += 1;
		
		status = true;
	}break;
	case 0x9C:
	{
		GPR[(int)EGPR::RSP] -= 8;
		*(UINT16*)GPR[(int)EGPR::RSP] = *(UINT16*)&FLAGS;
		RIP++;
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
		printf("PUSH");

	return status;
}

