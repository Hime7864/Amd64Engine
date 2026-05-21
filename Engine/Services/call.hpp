#pragma once

bool AssemblyState::service_call()
{
	auto status = false;


	switch (*RIP)
	{
	case 0xE8:
	{
		auto imm = *(INT32*)&RIP[1];
		GPR[(int)EGPR::RSP] -= 8;
		*(UINT64*)GPR[(int)EGPR::RSP] = (UINT64)(RIP + 5);
		Advancement = (UINT64)RIP + 5;
		RIP += imm + 5;
		status = true;
	}break;
	case 0xFF:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 2:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				GPR[(int)EGPR::RSP] -= 8;
				*(UINT64*)GPR[(int)EGPR::RSP] = (UINT64)RIP;
				Advancement = (UINT64)RIP;
				RIP = (BYTE*)*(UINT64*)ptr;
				status = true;
			}
		}break;
		case 3:
		{
		}break;
		}
	}break;
	};
	

	if (status)
		printf("Call");

	return status;
}

