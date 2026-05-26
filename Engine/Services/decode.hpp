#pragma once

#include <cstdio>

bool AssemblyState::decode_mnemonic()
{
	bool status = false;
	Prefix = { 0 };
	// skip NOPs
	switch (*RIP)
	{
	case 0x90:
		//printf("nop\n");
		RIP++;
		return true;
	};

	// repeated prefixes
	switch (*RIP)
	{
	case 0xF0:
	{
		RIP++;
		Prefix.LOCK = 1;
	}break;
	case 0xF2:
	{
		RIP++;
		Prefix.RepeatedNE = 1;
	}break;
	case 0xF3:
	{
		RIP++;
		Prefix.Repeated = 1;
	}break;
	case 0x9B:
	{
		//printf("Prefix 9B, Aborting\n");
		return false;
	}break;
	};

	// segment override prefixes
	switch (*RIP)
	{
	case 0x26:
	{
		Prefix.ES = 1;
		RIP++;
	}break;
	case 0x2E:
	{
		Prefix.CS = 1;
		RIP++;
	}break;
	case 0x36:
	{
		Prefix.SS = 1;
		RIP++;
	}break;
	case 0x3E:
	{
		Prefix.DS = 1;
		RIP++;
	}break;
	case 0x64:
	{
		Prefix.FS = 1;
		RIP++;
	}break;
	case 0x65:
	{
		Prefix.GS = 1;
		RIP++;
	}break;
	};

	// operand and address size override prefixes
	switch (*RIP)
	{
	case 0x66:
	{
		Prefix.OperandSize = 1;//32-16 bit
		RIP++;
	}break;
	case 0x67:
	{
		Prefix.AddressSize = 1;//64-32 bit
		RIP++;
	}break;
	};

	// REX prefix
	switch (*RIP)
	{
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
	case 0x48:
	case 0x49:
	case 0x4A:
	case 0x4B:
	case 0x4C:
	case 0x4D:
	case 0x4E:
	case 0x4F:
	{
		Prefix.W = (*RIP >> 3) & 1;
		Prefix.R = (*RIP >> 2) & 1;
		Prefix.X = (*RIP >> 1) & 1;
		Prefix.B = (*RIP >> 0) & 1;
		RIP++;
	}break;
	default:
		break;
	};

	// third pass for primary opcode
	switch (*RIP)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	{
		//printf("add\n");
		status = service_add();
	}break;
	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x0B:
	case 0x0C:
	case 0x0D:
	{
		//printf("or\n");
		status = service_or();
	}break;
	case 0x0F:
	{
		switch (RIP[1])
		{
		case 0x05:
		{
			//printf("syscall\n");
			status = service_syscall();
		}break;
		case 0x10:
		case 0x11:
		case 0x12:
		{
			//printf("movups\n");
			status = service_mov();
		}break;
		case 0x1F:
		{
			auto modrm = (MODRM*)(&RIP[2]);
			switch (modrm->Mode)
			{
			case EMODE::MEM_0_BIT_DISP:
			case EMODE::MEM_8_BIT_DISP:
			case EMODE::MEM_32_BIT_DISP:
			{
				//printf("nop\n");
				RIP++;
				GetDisplacementPtr();
				return true;
			}break;
			case EMODE::REG_TO_REG:
			{
				//printf("nop\n");
				RIP += 2;
				return true;
			}break;
			};
		}break;
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B:
		case 0x4C:
		case 0x4D:
		case 0x4E:
		case 0x4F:
		{
			//printf("cmov\n");
			status = service_mov();
		}break;
		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:
		case 0x8E:
		case 0x8F:
		{
			//printf("jmpcc\n");
			status = service_jmp();
		}break;
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9a:
		case 0x9b:
		case 0x9c:
		case 0x9d:
		case 0x9e:
		case 0x9f:
		{
			status = service_setcc();
		}break;
		case 0xA3:
		{
			//printf("bt\n");
			status = service_bt();
		}break;
		case 0xAB:
		{
			//printf("bts\n");
			status = service_bt();
		}break;
		case 0xB3:
		{
			//printf("btr\n");
			status = service_bt();
		}break;
		case 0xB6:
		case 0xB7:
		{
			//printf("movzx\n");
			status = service_mov();
		}break;
		case 0xBA:
		{
			auto modrm = (MODRM*)(&RIP[2]);
			switch (modrm->Register)
			{
			case 4:
			{
				//printf("bt\n");
				status = service_bt();
			}break;
			case 5:
			{
				//printf("bts\n");
				status = service_bt();
			}break;
			case 6:
			{
				//printf("btr\n");
				status = service_bt();
			}break;
			case 7:
			{
				//printf("btc\n");
				status = service_bt();
			}break;
			};
		}break;
		case 0xBB:
		{
			//printf("btc\n");
			status = service_bt();
		}break;
		}
	}break;
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	{
		status = service_adc();
	}break;
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	{
		status = service_sbb();
	}break;
	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
	case 0x25:
	{
		//printf("and\n");
		status = service_and();
	}break;
	case 0x28:
	case 0x29:
	case 0x2A:
	case 0x2B:
	case 0x2C:
	case 0x2D:
	{
		//printf("sub\n");
		status = service_sub();
	}break;
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	{
		//printf("xor\n");
		status = service_xor();
	}break;
	case 0x38:
	case 0x39:
	case 0x3A:
	case 0x3B:
	case 0x3C:
	case 0x3D:
	{
		//printf("cmp\n");
		status = service_cmp();
	}break;
	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
	{
		//printf("push\n");
		status = service_push();
	}break;
	case 0x58:
	case 0x59:
	case 0x5A:
	case 0x5B:
	case 0x5C:
	case 0x5D:
	case 0x5E:
	case 0x5F:
	{
		//printf("pop\n");
		status = service_pop();
	}break;
	case 0x63:
	{
		//printf("movsxd\n");
		status = service_mov();
	}break;
	case 0x68:
	{
		status = service_push();
	}break;
	case 0x69:
	{
		status = service_mul();
	}break;
	case 0x6A:
	{
		status = service_push();
	}break;
	case 0x6B:
	{
		status = service_mul();
	}break;
	case 0x6C:
	case 0x6D:
	{
		status = service_ins();
	}break;
	case 0x6E:
	case 0x6F:
	{
		status = service_outs();
	}break;
	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x76:
	case 0x77:
	case 0x78:
	case 0x79:
	case 0x7A:
	case 0x7B:
	case 0x7C:
	case 0x7D:
	case 0x7E:
	case 0x7F:
	{
		//printf("jmpcc\n");
		status = service_jmp();
	}break;
	case 0x80:
	case 0x81:
	case 0x83:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		{
			//printf("add\n");
			status = service_add();
		}break;
		case 1:
		{
			//printf("or\n");
			status = service_or();
		}break;
		case 4:
		{
			//printf("and\n");
			status = service_and();
		}break;
		case 5:
		{
			//printf("sub\n");
			status = service_sub();
		}break;
		case 6:
		{
			//printf("xor\n");
			status = service_xor();
		}break;
		case 7:
		{
			//printf("cmp\n");
			status = service_cmp();
		}break;
		};
	}break;
	case 0x84:
	case 0x85:
	{
		//printf("test\n");
		status = service_test();
	}break;
	case 0x88:
	case 0x89:
	case 0x8A:
	case 0x8B:
	case 0x8C:
	{
		//printf("mov\n");
		status = service_mov();
	}break;
	case 0x8D:
	{
		//printf("lea\n");
		status = service_lea();
	}break;
	case 0x8E:
	{
		//printf("mov\n");
		status = service_mov();
	}break;
	case 0x8F:
	{
		//printf("pop\n");
		status = service_pop();
	}break;
	case 0x9C:
	{
		//printf("pushf\n");
		status = service_push();
	}break;
	case 0x9D:
	{
		//printf("popf\n");
		status = service_pop();
	}break;
	case 0xA0:
	case 0xA1:
	case 0xA2:
	case 0xA3:
	case 0xA4:
	case 0xA5:
	{
		//printf("movs\n");
		status = service_mov();
	}break;
	case 0xA8:
	case 0xA9:
	{
		//printf("test\n");
		status = service_test();
	}break;
	case 0xAB:
	{
		//printf("stosd\n");
		status = service_stosd();
	}break;
	case 0xB0:
	case 0xB1:
	case 0xB2:
	case 0xB3:
	case 0xB4:
	case 0xB5:
	case 0xB6:
	case 0xB7:
	case 0xB8:
	case 0xB9:
	case 0xBA:
	case 0xBB:
	case 0xBC:
	case 0xBD:
	case 0xBE:
	case 0xBF:
	{
		//printf("mov\n");
		status = service_mov();
	}break;
	case 0xC0:
	case 0xC1:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		{
			//printf("rol\n");
			status = service_rol();
		}break;
		case 1:
		{
			//printf("ror\n");
			status = service_ror();
		}break;
		case 2:
		{
			//printf("rcl\n");
			status = service_rcl();
		}break;
		case 3:
		{
			//printf("rcr\n");
			status = service_rcr();
		}break;
		case 4:
		{
			//printf("shl\n");
			status = service_shl();
		}break;
		case 5:
		{
			//printf("shr\n");
			status = service_shr();
		}break;
		case 6:
		{
			//printf("sal\n");
			status = service_shl();
		}break;
		case  7:
		{
			//printf("sar\n");
			status = service_sar();
		}break;
		};
	}break;
	case 0xC3:
	{
		//printf("ret\n");
		status = service_ret();
	}break;
	case 0xC6:
	case 0xC7:
	{
		//printf("mov\n");
		status = service_mov();
	}break;
	case 0xD0:
	case 0xD1:
	case 0xD2:
	case 0xD3:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		{
			//printf("rol\n");
			status = service_rol();
		}break;
		case 1:
		{
			//printf("ror\n");
			status = service_ror();
		}break;
		case 2:
		{
			//printf("rcl\n");
			status = service_rcl();
		}break;
		case 3:
		{
			//printf("rcr\n");
			status = service_rcr();
		}break;
		case 4:
		{
			//printf("shl\n");
			status = service_shl();
		}break;
		case 5:
		{
			//printf("shr\n");
			status = service_shr();
		}break;
		case 6:
		{
			//printf("sal\n");
			status = service_shl();
		}break;
		case  7:
		{
			//printf("sar\n");
			status = service_sar();
		}break;
		};
	}break;
	case 0xE8:
	{
		//printf("call\n");
		status = service_call();
	}break;
	case 0xE9:
	case 0xEB:
	{
		//printf("jmp\n");
		status = service_jmp();
	}break;
	case 0xF6:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		case 1:
		{
			//printf("test\n");
			status = service_test();
		}break;
		};
	}break;
	case 0xF7:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		case 1:
		{
			//printf("test\n");
			status = service_test();
		}break;
		};
	}break;
	case 0xFE:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		{
			//printf("inc\n");
			status = service_inc();
		}break;
		case 1:
		{
			//printf("dec\n");
			status = service_dec();
		}break;
		}
	}break;
	case 0xFF:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
		{
			//printf("inc\n");
			status = service_inc();
		}break;
		case 1:
		{
			//printf("dec\n");
			status = service_dec();
		}break;
		case 2:
		case 3:
		{
			//printf("call\n");
			status = service_call();
		}break;
		case 4:
		case 5:
		{
			//printf("jmp\n");
			status = service_jmp();
		}break;
		case 6:
		{
			//printf("push\n");
			status = service_push();
		}break;
		}
	}break;
	};

	

	if (RIP == 0x0)
		return true;

	if (!status)
	{
		printf("Unknown instruction: %02X %02X\n", RIP[0], RIP[1]);
	}

	return status;
}

