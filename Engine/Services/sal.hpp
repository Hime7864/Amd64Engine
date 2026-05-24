#pragma once

bool AssemblyState::service_sal()
{
	bool status = false;

	switch (*RIP)
	{
	case 0xC0:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto dest = *(UINT8*)ptr;
			auto src = RIP[2];
		}
	}break;
	case 0xC1:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			if (Prefix.W)
			{
				auto dest = *(UINT64*)ptr;
				auto src = RIP[2];
			}
			else if (!Prefix.OperandSize)
			{
				auto dest = *(UINT32*)ptr;
				auto src = RIP[2]; 
			}
			else
			{
				auto dest = *(UINT16*)ptr;
				auto src = RIP[2];
			}
		}
	}break;
	case 0xD0:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto dest = *(UINT8*)ptr;
			auto src = 1;
		}
	}break;
	case 0xD1:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			if (Prefix.W)
			{
				auto dest = *(UINT64*)ptr;
				auto src = 1;
			}
			else if (!Prefix.OperandSize)
			{
				auto dest = *(UINT32*)ptr;
				auto src = 1; 
			}
			else
			{
				auto dest = *(UINT16*)ptr;
				auto src = 1;
			}
		}
	}break;
	case 0xD2:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto dest = *(UINT8*)ptr;
			auto src = *(UINT8*)&GPR[(int)EGPR::RCX];
		}
	}break;
	case 0xD3:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			if (Prefix.W)
			{
				auto dest = *(UINT64*)ptr;
				auto src = (UINT64) * (UINT8*)&GPR[(int)EGPR::RCX];
			}
			else if (!Prefix.OperandSize)
			{
				auto dest = *(UINT32*)ptr;
				auto src = (UINT32) * (UINT8*)&GPR[(int)EGPR::RCX]; 
			}
			else
			{
				auto dest = *(UINT16*)ptr;
				auto src = (UINT16) * (UINT8*)&GPR[(int)EGPR::RCX];
			}
		}
	}break;
	}
	
	return status;
}

