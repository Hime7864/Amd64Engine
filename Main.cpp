#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Binary/Binary.hpp"
#include "Disasm/ZydisWrapper.hpp"

struct IRInstruction
{

};

struct Instruction
{
	std::vector<IRInstruction> ir;
	uint64_t rip = 0;
};

struct BasicBlock
{
	BasicBlock* pathA = nullptr;
	BasicBlock* pathB = nullptr;
	std::vector<Instruction> amd64;
	bool scan_complete = false;
	bool scan_paused = false;
};

class ControlFlowGraph
{
public:

	std::vector<BasicBlock> workload;
};


int main()
{
	auto binary = Binary::Load(L"C:\\Users\\Admin\\Desktop\\EfiLoader.exe");
	if (!binary)
		return -1;

	auto start_rip = binary->GetStartup();
	if (!start_rip)
		return -1;

	delete binary;
	return 0;
}
