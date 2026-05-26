#pragma once

bool AssemblyState::service_lea()
{
	bool status = false;
	auto modrm = (MODRM*)(&RIP[1]);
	auto ptr = GetDisplacementPtr();
	if (ptr)
	{
		auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
		if (Prefix.W)
		{
			GPR[modrm_register] = ptr;
		}
		else if (!Prefix.OperandSize)
		{
			
			GPR[modrm_register] = (UINT64)(UINT32)ptr;
		}
		else
		{
			*(UINT32*)&GPR[modrm_register] = (UINT32)(UINT16)ptr;
		}
		status = true;
	}

	return status;
}