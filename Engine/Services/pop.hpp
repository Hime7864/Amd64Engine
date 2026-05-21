#pragma once

bool AssemblyState::service_pop()
{
	bool status = false;

	switch (*RIP)
	{
	case 0x58:
	case 0x59:
	case 0x5A:
	case 0x5B:
	case 0x5C:
	case 0x5D:
	case 0x5E:
	case 0x5F:
	{
		auto reg = *RIP & 0x7;
		if (Prefix.B)
			reg += 8;

		if (Prefix.OperandSize)
		{
			GPR[reg] = *(UINT16*)GPR[(int)EGPR::RSP];
			GPR[(int)EGPR::RSP] += 2;
		}
		else
		{
			GPR[reg] = *(UINT64*)GPR[(int)EGPR::RSP];
			GPR[(int)EGPR::RSP] += 8;
		}
		

		RIP += 1;

		status = true;
	}break;
	case 0x8F:
	{
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			if (Prefix.OperandSize)
			{
				*(UINT16*)ptr = *(UINT16*)GPR[(int)EGPR::RSP];
				GPR[(int)EGPR::RSP] += 2;
			}
			else
			{
				*(UINT64*)ptr = *(UINT64*)GPR[(int)EGPR::RSP];
				GPR[(int)EGPR::RSP] += 8;
			}
			status = true;
		}
	}break;
	case 0x9D:
	{
		*(UINT16*)&FLAGS = *(UINT16*)GPR[(int)EGPR::RSP];
		GPR[(int)EGPR::RSP] += 8;
		RIP++;
		status = true;	
	}break;
	};

	if(status)
		printf("Pop\n");

	return status;
}

