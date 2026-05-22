#include <Windows.h>
#include <stdio.h>

#include "Engine.hpp"

void test()
{
	//return VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
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
	auto start_rsp = engine->GetGPR((int)EGPR::RSP);
	engine->SetRip((PVOID)test);

	int counter = 0;
	while (engine->step())
	{
		printf("RAX %p ", engine->GetGPR((int)EGPR::RAX));
		printf("RCX %p ", engine->GetGPR((int)EGPR::RCX));
		printf("RDX %p ", engine->GetGPR((int)EGPR::RDX));
		printf("RBX %p \n", engine->GetGPR((int)EGPR::RBX));
		printf("RSP %p ", engine->GetGPR((int)EGPR::RSP));
		printf("RBP %p ", engine->GetGPR((int)EGPR::RBP));
		printf("RSI %p ", engine->GetGPR((int)EGPR::RSI));
		printf("RDI %p \n", engine->GetGPR((int)EGPR::RDI));
		printf("R8  %p ", engine->GetGPR((int)EGPR::R8));
		printf("R9  %p ", engine->GetGPR((int)EGPR::R9));
		printf("R10 %p ", engine->GetGPR((int)EGPR::R10));
		printf("R11 %p \n", engine->GetGPR((int)EGPR::R11));
		printf("R12 %p ", engine->GetGPR((int)EGPR::R12));
		printf("R13 %p ", engine->GetGPR((int)EGPR::R13));
		printf("R14 %p ", engine->GetGPR((int)EGPR::R14));
		printf("R15 %p \n", engine->GetGPR((int)EGPR::R15));
		printf("Nxt ");
		for (int i = 0; i < 10; i++)
			printf("%02X ", ((UINT8*)engine->GetRip())[i]);
		printf("\n\n");
		printf("idx: %d\n", counter);
		printf("RIP: %p\n", engine->GetRip());
		counter++;
	}

	printf("Executed %d instructions\n", counter);


	return 0;
}
