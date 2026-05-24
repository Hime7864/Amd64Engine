#pragma once

bool AssemblyState::service_ret()
{
	auto status = false;
	RIP = (BYTE*)*(UINT64*)GPR[(int)EGPR::RSP];
	GPR[(int)EGPR::RSP] += 8;
	status = true;
	return status;
}

