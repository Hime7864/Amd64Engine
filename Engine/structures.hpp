#pragma once

#include <Windows.h>

struct EFLAGS
{
	BYTE CF : 1;
	BYTE reserved_1 : 1;
	BYTE PF : 1;
	BYTE reserved_2 : 1;
	BYTE AF : 1;
	BYTE reserved_3 : 1;
	BYTE ZF : 1;
	BYTE SF : 1;
	BYTE TF : 1;
	BYTE IF : 1;
	BYTE DF : 1;
	BYTE OF : 1;
};

enum struct EGPR : BYTE
{
	RAX = 0,
	RCX = 1,
	RDX = 2,
	RBX = 3,
	RSP = 4,
	RBP = 5,
	RSI = 6,
	RDI = 7,
	R8 = 8,
	R9 = 9,
	R10 = 10,
	R11 = 11,
	R12 = 12,
	R13 = 13,
	R14 = 14,
	R15 = 15
};

enum struct EMODE : BYTE
{
	MEM_0_BIT_DISP = 0,
	MEM_8_BIT_DISP = 1,
	MEM_32_BIT_DISP = 2,
	REG_TO_REG = 3
};

struct MODRM
{
	BYTE RegisterMemory : 3;
	BYTE Register : 3;
	EMODE Mode : 2;
};

struct MNEMONICPREFIX
{
	INT16 B : 1;
	INT16 X : 1;
	INT16 R : 1;
	INT16 W : 1;
	INT16 CS : 1;
	INT16 SS : 1;
	INT16 DS : 1;
	INT16 ES : 1;
	INT16 FS : 1;
	INT16 GS : 1;
	INT16 LOCK : 1;
	INT16 OperandSize : 1;
	INT16 AddressSize : 1;
	INT16 Repeated : 1;
};
