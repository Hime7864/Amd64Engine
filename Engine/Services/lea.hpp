#pragma once

bool AssemblyState::service_lea()
{
	bool status = false;
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
				GPR[modrm_register] = ptr;
			}
			else
			{
				if (Prefix.OperandSize)
				{
					*(WORD*)&GPR[modrm_register] = (WORD)ptr;
				}
				else
				{
					GPR[modrm_register] = 0;
					*(DWORD*)&GPR[modrm_register] = (DWORD)ptr;
				}
			}
			status = true;
		}
	}break;
	};

	if (status)
		printf("Load Effective Address");

	return status;
}