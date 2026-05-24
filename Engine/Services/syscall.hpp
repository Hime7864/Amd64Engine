#pragma once

int syscall_number;

class FnSyscall
{
public:
    template <typename Ret, typename... Args>
	static __attribute__((naked)) Ret invoke(Args... args)
    {
		__asm {
			mov r10, rcx
			mov eax, [syscall_number]
			syscall
			ret
		}
    }
};

bool AssemblyState::service_syscall()
{
	bool status = false;

	FnSyscall syscall;
	printf("Syscall: %d\n", (int)GPR[(int)EGPR::RAX]);
	syscall_number = GPR[(int)EGPR::RAX];
	GPR[(int)EGPR::RAX] = FnSyscall::invoke<UINT64>(
		GPR[(int)EGPR::R10],
		GPR[(int)EGPR::RDX],
		GPR[(int)EGPR::R8],
		GPR[(int)EGPR::R9],
		*(UINT64*)(GPR[(int)EGPR::RSP] + 0x28),
		*(UINT64*)(GPR[(int)EGPR::RSP] + 0x30),
		*(UINT64*)(GPR[(int)EGPR::RSP] + 0x38),
		*(UINT64*)(GPR[(int)EGPR::RSP] + 0x40),
		*(UINT64*)(GPR[(int)EGPR::RSP] + 0x48),
		*(UINT64*)(GPR[(int)EGPR::RSP] + 0x50)
	);

	RIP += 2;
	status = true;

	return status;
}

