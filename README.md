# x86 Emulator / Micro-Op Decoder

**Work in Progress** — Currently only targeting CPL > 0 opcodes. Actively refactoring the codebase. Currently working on another project however I'm now thinking on changing the approach once more we will see

## Current Goal

Get the emulator running first, then shift the architecture from a traditional monolithic emulator to a **micro-op style design**.

## Future Vision

I want to break instructions down into smaller, more basic operations — similar to how real CPUs decode complex instructions into micro-ops executed out of order.

The long-term goal is to introduce a simple **Intermediate Language (IL)** with a very limited instruction set.

## Example: ROL Instruction

Instead of handling complex instructions in one large block like this:

```cpp
UINT8 count = RIP[0];
auto tempCount = (count & 0x1F) % 9;
auto countMask = 0x1F;

while (tempCount)
{
	auto tempCF = get_msb(*(UINT8*)ptr, 8);
	*(UINT8*)ptr = (*(UINT8*)ptr * 2) + tempCF;
	tempCount--;
}

if ((count & countMask) == 1)
{
	FLAGS.OF = get_msb(*(UINT8*)ptr, 8) ^ FLAGS.CF;
}
```
The plan is to decompose it into many smaller, simpler micro-operations while preserving identical behavior.
##Why This Approach?
Simpler micro-operations should lead to easier and more effective optimizations. Even if the micro-op layer is more computationally expensive at first, the reduced complexity per operation creates clearer optimization paths.

This current version is essentially a simplified Unicorn Engine without using Capstone or any external disassembler/assembler. I believe directly executing opcodes in their raw decoded form is inefficient, especially since many instructions share the same core semantics (e.g., many forms of MOV) but require separate handlers.
