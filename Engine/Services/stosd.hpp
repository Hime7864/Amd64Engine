#pragma once

bool AssemblyState::service_stosd()
{
	auto status = false;
	

	int count = 1;
	if (Prefix.Repeated)
		count = (int)GPR[(int)EGPR::RCX];

	for (int i = 0; i < count; i++)
	{
		auto ptr = GPR[(int)EGPR::RDI];

		if (Prefix.GS)
			ptr += GsBase;
		else if (Prefix.FS)
			ptr += FsBase;

		if (Prefix.OperandSize)
		{
			*(UINT16*)ptr = (UINT16)GPR[(int)EGPR::RAX];
			GPR[(int)EGPR::RDI] += 2;
		}
		else
		{
			*(UINT32*)ptr = (UINT32)GPR[(int)EGPR::RAX];
			GPR[(int)EGPR::RDI] += 4;
		}
	}
	
	status = true;
	RIP += 1;

	return status;
}

