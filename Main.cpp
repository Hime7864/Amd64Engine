#include <Windows.h>
#include <stdio.h>

#include "Engine.hpp"

void test()
{
	printf("Hello world\n");
	return;
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
		printf("%p(RAX) ", engine->GetGPR((int)EGPR::RAX));
		printf("%p(RCX) ", engine->GetGPR((int)EGPR::RCX));
		printf("%p(RDX) ", engine->GetGPR((int)EGPR::RDX));
		printf("%p(RBX) \n", engine->GetGPR((int)EGPR::RBX));
		printf("%p(RSP) ", engine->GetGPR((int)EGPR::RSP));
		printf("%p(RBP) ", engine->GetGPR((int)EGPR::RBP));
		printf("%p(RSI) ", engine->GetGPR((int)EGPR::RSI));
		printf("%p(RDI) \n", engine->GetGPR((int)EGPR::RDI));
		printf("%p( R8) ", engine->GetGPR((int)EGPR::R8));
		printf("%p( R9) ", engine->GetGPR((int)EGPR::R9));
		printf("%p(R10) ", engine->GetGPR((int)EGPR::R10));
		printf("%p(R11) \n", engine->GetGPR((int)EGPR::R11));
		printf("%p(R12) ", engine->GetGPR((int)EGPR::R12));
		printf("%p(R13) ", engine->GetGPR((int)EGPR::R13));
		printf("%p(R14) ", engine->GetGPR((int)EGPR::R14));
		printf("%p(R15) \n", engine->GetGPR((int)EGPR::R15));
		printf("idx : %d instructions\nNEXT: ", counter);
		for (int i = 0; i < 10; i++)
			printf("%02X ", ((UINT8*)engine->GetRip())[i]);
		printf("\n\n");
		counter++;
	}

	printf("Executed %d instructions\n", counter);


	return 0;
}
