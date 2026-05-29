#pragma once

#include <cstdio>

void AssemblyState::decode_prefixes()
{
    // repeated prefixes
	auto start_rip = (UINT64)RIP;

    if (Prefix.CS || Prefix.SS || Prefix.DS || Prefix.ES)
    {
        Prefix.FS = 0;
        Prefix.GS = 0;
        switch (*RIP)
        {
        case 0x26:
        {
            Prefix.ES = 1;
            Prefix.CS = 0;
            Prefix.SS = 0;
            Prefix.DS = 0;
            
            RIP++;
        }break;
        case 0x2E:
        {
            Prefix.ES = 0;
            Prefix.CS = 1;
            Prefix.SS = 0;
            Prefix.DS = 0;
            
            RIP++;
        }break;
        case 0x36:
        {
            Prefix.ES = 0;
            Prefix.CS = 0;
            Prefix.SS = 1;
            Prefix.DS = 0;
            
            RIP++;
        }break;
        case 0x3E:
        {
            Prefix.ES = 0;
            Prefix.CS = 0;
            Prefix.SS = 0;
            Prefix.DS = 1;
            RIP++;
        }break;
        }
    }
    else
    {
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
            Prefix.GS = 0;
            Prefix.FS = 1;
            RIP++;
        }break;
        case 0x65:
        {
            Prefix.GS = 0;
            Prefix.GS = 1;
            RIP++;
        }break;
        }
    }

    switch (*RIP)
    {
    case 0x66:
    {
        Prefix.OperandSize = 1;
        RIP++;
    }break;
    case 0x67:
    {
        Prefix.AddressSize = 1;
        RIP++;
    }break;
    case 0xF0:
    {
        Prefix.LOCK = 1;
        RIP++;
    }break;
    case 0xF2:
    {
        Prefix.Repeated = 0;
        Prefix.RepeatedNE = 1;
        RIP++;
    }break;
    case 0xF3:
    {
        Prefix.Repeated = 1;
        Prefix.RepeatedNE = 0;
        RIP++;
    }break;
    }

    if(start_rip != (UINT64)RIP)
        decode_prefixes();

    if (*RIP >= 0x40 && *RIP < 0x50)
    {
        Prefix.W = (*RIP >> 3) & 1;
        Prefix.R = (*RIP >> 2) & 1;
        Prefix.X = (*RIP >> 1) & 1;
        Prefix.B = (*RIP >> 0) & 1;
        RIP++;
    }

    return;
}

bool AssemblyState::decode_mnemonic()
{
	bool status = false;
	Prefix = { 0 };

    decode_prefixes();

    if (*RIP == 0x0F)
    {
        switch (RIP[1])
        {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        case 0x2E:
        case 0x2F:
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
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
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
        case 0x67:
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6E:
        case 0x6F:
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
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA4:
        case 0xA5:
        case 0xA6:
        case 0xA7:
        case 0xA8:
        case 0xA9:
        case 0xAA:
        case 0xAB:
        case 0xAC:
        case 0xAD:
        case 0xAE:
        case 0xAF:
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
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            break;
        }
    }
    else
    {
        switch (*RIP)
        {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        {
            status = service_add();
        }break;
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        {
            status = service_or();
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
            status = service_and();
        }break;
        case 0x28:
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        {
            status = service_sub();
        }break;
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        {
            status = service_xor();
        }break;
        case 0x38:
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        {
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
            status = service_pop();
        }break;
        case 0x63:
        {
            status = service_movsxd();
        }break;
        case 0x68:
        {
            status = service_push();
        }break;
        case 0x69:
        {
            status = service_imul();
        }break;
        case 0x6A:
        {
            status = service_push();
        }break;
        case 0x6B:
        {
            status = service_imul();
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
            status = service_jcc();
        }break;
        case 0x80:
        case 0x81:
        {
            auto modrm = (MODRM*)(&RIP[1]);
            switch (modrm->Register)
            {
            case 0:
            {
                status = service_add();
            }break;
            case 1:
            {
                status = service_or();
            }break;
            case 2:
            {
                status = service_adc();
            }break;
            case 3:
            {
                status = service_sbb();
            }break;
            case 4:
            {
                status = service_and();
            }break;
            case 5:
            {
                status = service_sub();
            }break;
            case 6:
            {
                status = service_xor();
            }break;
            case 7:
            {
                status = service_cmp();
            }break;
            }
        }break;
        case 0x83:
        {
            auto modrm = (MODRM*)(&RIP[1]);
            switch (modrm->Register)
            {
            case 0:
            {
                status = service_add();
            }break;
            case 1:
            {
                status = service_or();
            }break;
            case 2:
            {
                status = service_adc();
            }break;
            case 3:
            {
                status = service_sbb();
            }break;
            case 4:
            {
                status = service_and();
            }break;
            case 5:
            {
                status = service_sub();
            }break;
            case 6:
            {
                status = service_xor();
            }break;
            case 7:
            {
                status = service_cmp();
            }break;
            }
        }break;
        case 0x84:
        case 0x85:
        {
            status = service_test();
        }break;
        case 0x86:
        case 0x87:
        {
            status = service_xchg();
        }break;
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        {
            status = service_mov();
        }break;
        case 0x8D:
        {
            status = service_lea();
        }break;
        case 0x8E:
        {
            status = service_mov();
        }break;
        case 0x8F:
        {
			auto modrm = (MODRM*)(&RIP[1]);
            if(modrm->Register == 0)
                status = service_pop();
        }break;
        case 0x90:
        {
            RIP++;
            status = true;
        }break;
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
        {
            status = service_xchg();
        }break;
        case 0x98:
        {
            status = service_cbw();
        }break;
        case 0x99:
        {
            status = service_cwd();
        }break;
        case 0x9C:
        {
            status = service_pushf();
        }break;
        case 0x9D:
        {
            status = service_popf();
        }break;
        case 0x9E:
        {
            status = service_sahf();
        }break;
        case 0x9F:
        {
            status = service_lahf();
        }break;
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        {
            status = service_mov();
        }break;
        case 0xA4:
        case 0xA5:
        {
            status = service_mova();
        }break;
        case 0xA6:
        case 0xA7:
        {
            status = service_cmps();
        }break;
        case 0xA8:
        case 0xA9:
        {
            status = service_test();
        }break;
        case 0xAA:
        case 0xAB:
        {
            status = service_stos();
        }break;
        case 0xAC:
        case 0xAD:
        {
            status = service_lods();
        }break;
        case 0xAE:
        case 0xAF:
        {
            status = service_scas();
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
                status = service_rol();
            }break;
            case 1:
            {
                status = service_ror();
            }break;
            case 2:
            {
                status = service_rcl();
            }break;
            case 3:
            {
                status = service_rcr();
            }break;
            case 4:
            {
                status = service_shl();
            }break;
            case 5:
            {
                status = service_shr();
            }break;
            case 6:
            {
                status = service_sal();
            }break;
            case 7:
            {
                status = service_sar();
            }break;
			}
        }break;
        case 0xC2:
        case 0xC3:
        {
			status = service_ret();
        }break;
        case 0xC6:
        case 0xC7:
        {
            status = service_mov();
        }break;
        case 0xC8:
        {
            status = service_enter();
        }break;
        case 0xC9:
        {
            status = service_leave();
        }break;
        case 0xCA:
        case 0xCB:
        {
            status = service_retf();
        }break;
        case 0xCC:
        case 0xCD:
        {
            status = service_int();
        }break;
        case 0xCE:
        {
            status = service_into();
        }break;
        case 0xCF:
        {
            status = service_iret();
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
                status = service_rol();
            }break;
            case 1:
            {
                status = service_ror();
            }break;
            case 2:
            {
                status = service_rcl();
            }break;
            case 3:
            {
                status = service_rcr();
            }break;
            case 4:
            {
                status = service_shl();
            }break;
            case 5:
            {
                status = service_shr();
            }break;
            case 6:
            {
                status = service_sal();
            }break;
            case 7:
            {
                status = service_sar();
            }break;
            }
        }break;
        case 0xD7:
        {
            status = service_xlat();
        }break;
        case 0xD8:
        {
            auto modrm = (MODRM*)(&RIP[1]);
            switch (modrm->Register)
            {
            case 0:
            {
                status = service_fadd();
            }break;
            case 1:
            {
                status = service_fmul();
            }break;
            case 2:
            {
                status = service_fcom();
            }break;
            case 3:
            {
                status = service_fcomp();
            }break;
            case 4:
            {
                status = service_fsub();
            }break;
            case 5:
            {
                status = service_fsubr();
            }break;
            case 6:
            {
                status = service_fdiv();
            }break;
            case 7:
            {
                status = service_fdivr();
            }break;
            };
        }break;
        case 0xD9:
        {
            auto modrm = (MODRM*)(&RIP[1]);
            switch (modrm->Register)
            {
            case 0:
            {
                status = service_flw();
            }break;
            case 1:
            {
                status = service_fxch();
            }break;
            case 2:
            {
                if (RIP[1] == 0xD0)
                {
                    RIP += 2;
                    status = true;
                }
                else
                {
                    status = service_fst();
                }
            }break;
            case 3:
            {
                status = service_fstp();
            }break;
            case 4:
            {
                status = service_fsub();
            }break;
            case 5:
            {
                status = service_fsubr();
            }break;
            case 6:
            {
                status = service_fdiv();
            }break;
            case 7:
            {
                status = service_fdivr();
            }break;
            };
        }break;
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            break;
        }
    }

	

	if (RIP == 0x0)
		return true;

	if (!status)
	{
		printf("Unknown instruction: %02X %02X\n", RIP[0], RIP[1]);
	}

	return status;
}

