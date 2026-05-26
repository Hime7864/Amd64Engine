#include <Windows.h>
#include <stdio.h>

#include "Engine.hpp"

void test()
{
	auto ptr = VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	VirtualFree(ptr, 0x1000, MEM_RELEASE);
	printf("Hello world\n");
}

UINT64 __attribute__((naked)) __readgsbase()
{
	__asm {
		mov rax, gs:[0x30]
		ret
	}
}

int main()
{
	auto engine = new AssemblyState();

	engine->SetGPR((int)EGPR::RSP, (UINT64)VirtualAlloc(nullptr, 0x20000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE) + 0x10000);
	engine->SetGsBase(__readgsbase());
	engine->SetRip((PVOID)test);

	while (engine->step())
	{
		//engine->log_step();
	}


	return 0;
}
