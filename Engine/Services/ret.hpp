#pragma once

bool AssemblyState::service_ret()
{
	auto status = false;
	Advancement = (UINT64)RIP + 1;
	RIP = (BYTE*)*(UINT64*)GPR[(int)EGPR::RSP];
	GPR[(int)EGPR::RSP] += 8;
	status = true;
	if (status)
		printf("Return");
	return status;
}

