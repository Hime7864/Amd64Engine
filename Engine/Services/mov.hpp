#pragma once

bool AssemblyState::service_mov()
{
	bool status = false;
	switch (*RIP)
	{
	case 0x0F:
	{
		RIP++;
		bool state = false;
		switch (*RIP)
		{
		case 0x40:
			state = FLAGS.OF;
			break;
		case 0x41:
			state = !FLAGS.OF;
			break;
		case 0x42:
			state = FLAGS.CF;
			break;
		case 0x43:
			state = !FLAGS.CF;
			break;
		case 0x44:
			state = FLAGS.ZF;
			break;
		case 0x45:
			state = !FLAGS.ZF;
			break;
		case 0x46:
			state = FLAGS.CF || FLAGS.ZF;
			break;
		case 0x47:
			state = (!FLAGS.CF && !FLAGS.ZF);
			break;
		case 0x48:
			state = FLAGS.SF;
			break;
		case 0x49:
			state = !FLAGS.SF;
			break;
		case 0x4A:
			state = FLAGS.PF;
			break;
		case 0x4B:
			state = !FLAGS.PF;
			break;
		case 0x4C:
			state = FLAGS.SF != FLAGS.OF;
			break;
		case 0x4D:
			state = FLAGS.SF == FLAGS.OF;
			break;
		case 0x4E:
			state = FLAGS.ZF && (FLAGS.SF != FLAGS.OF);
			break;
		case 0x4F:
			state = !FLAGS.ZF && (FLAGS.SF == FLAGS.OF);
			break;
		};
		if (*RIP >= 0x40 && *RIP <= 0x4F)
		{
			auto modrm = (MODRM*)&RIP[1];
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				if (state)
				{
					auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
					if (Prefix.W)
					{
						GPR[modrm_register] = *(UINT64*)ptr;
					}
					else if (!Prefix.OperandSize)
					{
						GPR[modrm_register] = (UINT64)*(UINT32*)ptr;
					}
					else
					{
						*(UINT32*)&GPR[modrm_register] = (UINT32)*(UINT16*)ptr;
					}
				}
				status = true;
			}
		}
		else
		{
			switch (*RIP)
			{
			case 0x10:
			{
				auto modrm = (MODRM*)(&RIP[2]);
				auto ptr = GetDisplacementPtr();
				if (ptr)
				{
					auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

					if (Prefix.OperandSize)
					{
						XMM[modrm_register] = *(XMMWORD*)ptr;
					}
					else if (Prefix.RepeatedNE)
					{
						auto src = (XMMWORD*)ptr;
						XMM[modrm_register].u64.data[0] = src->u64.data[0];
					}
					else if (Prefix.Repeated)
					{

						auto src = (XMMWORD*)ptr;
						XMM[modrm_register].u32.data[0] = src->u32.data[0];
					}
					else
					{

						XMM[modrm_register] = *(XMMWORD*)ptr;
					}

					status = true;
				}
			}break;
			case 0x11:
			{
				auto modrm = (MODRM*)(&RIP[2]);
				auto ptr = GetDisplacementPtr();
				if (ptr)
				{
					auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;

					if (Prefix.OperandSize)
					{
						*(XMMWORD*)ptr = XMM[modrm_register];
					}
					else if (Prefix.RepeatedNE)
					{
						auto src = (XMMWORD*)ptr;
						src->u64.data[0] = XMM[modrm_register].u64.data[0];
					}
					else if (Prefix.Repeated)
					{

						auto src = (XMMWORD*)ptr;
						src->u32.data[0] = XMM[modrm_register].u32.data[0];
					}
					else
					{

						*(XMMWORD*)ptr = XMM[modrm_register];
					}

					status = true;
				}
			}break;
			case 0xB6:// r16/32/64	r/m8
			{
				auto modrm = (MODRM*)&RIP[1];
				auto ptr = GetDisplacementPtr();
				if (ptr)
				{
					auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
					if (Prefix.W)
					{
						GPR[modrm_register] = (UINT64)*(BYTE*)ptr;
					}
					else if (!Prefix.OperandSize)
					{
						GPR[modrm_register] = (UINT64)*(BYTE*)ptr;
					}
					else
					{
						*(UINT32*)&GPR[modrm_register] = (UINT32)*(BYTE*)ptr;
					}
					status = true;
				}
			}break;
			case 0xB7:// r16/32/64	r/m16
			{
				auto modrm = (MODRM*)&RIP[1];
				auto ptr = GetDisplacementPtr();
				if (ptr)
				{
					auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
					if (Prefix.W)
					{
						GPR[modrm_register] = (UINT64) *(UINT16*)ptr;
					}
					else if (!Prefix.OperandSize)
					{
						GPR[modrm_register] = (UINT64)*(UINT16*)ptr; 
					}
					else
					{
						*(UINT32*)&GPR[modrm_register] = (UINT32)*(UINT16*)ptr;
					}
					status = true;
				}
			}break;
			}
		}
	}break;
	case 0x63:// r32/64	r/m32
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			if (Prefix.W)
			{
				GPR[modrm_register] = (UINT64)(INT32)*(UINT32*)ptr;
			}
			else
			{
				GPR[modrm_register] = *(UINT32*)ptr;
			}
			status = true;
		}
	}break;
	case 0x88:// r/m8	r8
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			*(BYTE*)ptr = *(BYTE*)&GPR[modrm_register];
			status = true;
		}
	}break;
	case 0x89:// r/m16/32/64	r16/32/64
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			if (Prefix.W)
			{
				*(UINT64*)ptr = GPR[modrm_register];
			}
			else if (!Prefix.OperandSize)
			{
				*(UINT64*)ptr = (UINT64)*(UINT32*)&GPR[modrm_register];
			}
			else
			{
				*(UINT32*)ptr = (UINT32)*(UINT16*)&GPR[modrm_register];
			}
			status = true;
		}

	}break;
	case 0x8A:// r8	r/m8
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			*(BYTE*)&GPR[modrm_register] = *(BYTE*)ptr;
			status = true;
		}
	}break;
	case 0x8B://r16/32/64	r/m16/32/64
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			if (Prefix.W)
			{
				GPR[modrm_register] = *(UINT64*)ptr;
			}
			else if (!Prefix.OperandSize)
			{
				GPR[modrm_register] = (UINT64)*(UINT32*)ptr;
			}
			else
			{
				*(UINT32*)&GPR[modrm_register] = (UINT32)*(UINT16*)ptr;
			}
			status = true;
		}
	}break;
	case 0x8C:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 0:
			GPR[modrm->RegisterMemory] = 0x2B;
			break;
		case 1:
			GPR[modrm->RegisterMemory] = 0x33;
			break;
		case 2:
			GPR[modrm->RegisterMemory] = 0x2B;
			break;
		case 3:
			GPR[modrm->RegisterMemory] = 0x2B;
			break;
		case 4:
			GPR[modrm->RegisterMemory] = 0x53;
			break;
		case 5:
			GPR[modrm->RegisterMemory] = 0x2B;
			break;
		}
		RIP += 2;
		status = true;
	}break;
	case 0xA0:// AL	moffs8
	{
		auto ptr = *(UINT64*)(&RIP[1]);
		*(BYTE*)&GPR[(BYTE)EGPR::RAX] = *(BYTE*)ptr;
		RIP += 9;
		status = true;
	}break;
	case 0xA1:
	{
		auto moffs = *(UINT64*)(&RIP[1]);
		if (Prefix.W)
		{
			GPR[(BYTE)EGPR::RAX] = *(UINT64*)moffs;
		}
		else if (!Prefix.OperandSize)
		{
			GPR[(BYTE)EGPR::RAX] = (UINT64)*(UINT32*)moffs;
		}
		else
		{
			*(UINT32*)&GPR[(BYTE)EGPR::RAX] = (UINT32)*(UINT16*)moffs;
		}

		RIP += 9;
		status = true;
	}break;
	case 0xA2:
	{
		auto moffs = *(UINT64*)(&RIP[1]);
		*(BYTE*)moffs = (BYTE)GPR[(BYTE)EGPR::RAX];
		RIP += 9;
		status = true;
	}break;
	case 0xA3:
	{
		auto moffs = *(UINT64*)(&RIP[1]);
		if (Prefix.W)
		{
			*(UINT64*)moffs = GPR[(BYTE)EGPR::RAX];
		}
		else if (!Prefix.OperandSize)
		{
			*(UINT64*)moffs = (UINT64)*(UINT32*)&GPR[(BYTE)EGPR::RAX];
		}
		else
		{
			*(UINT32*)moffs = (UINT32)*(UINT16*)&GPR[(BYTE)EGPR::RAX];
		}

		RIP += 9;
		status = true;
	}break;
	case 0xA4:
	{
		auto src = GPR[(BYTE)EGPR::RSI];
		auto dst = GPR[(BYTE)EGPR::RDI];
		auto count = GPR[(BYTE)EGPR::RCX];
		memcpy((PVOID)dst, (PVOID)src, count);
		RIP++;
		status = true;
	}break;
	case 0xA5:
	{
		auto src = GPR[(BYTE)EGPR::RSI];
		auto dst = GPR[(BYTE)EGPR::RDI];
		auto count = GPR[(BYTE)EGPR::RCX];

		if (Prefix.W)
		{
			memcpy((PVOID)dst, (PVOID)src, count * 8);
		}
		else
		{
			if (Prefix.OperandSize)
			{
				memcpy((PVOID)dst, (PVOID)src, count * 2);
			}
			else
			{
				memcpy((PVOID)dst, (PVOID)src, count * 4);
			}
		}
		RIP++;
		status = true;
	}break;
	case 0xB0:
	case 0xB1:
	case 0xB2:
	case 0xB3:
	case 0xB4:
	case 0xB5:
	case 0xB6:
	case 0xB7:
	{
		auto reg = *RIP & 0x7;
		if (Prefix.R)
			reg += 8;
		*(BYTE*)&GPR[reg] = RIP[1];
		RIP += 2;
		status = true;
	}break;
	case 0xB8:
	case 0xB9:
	case 0xBA:
	case 0xBB:
	case 0xBC:
	case 0xBD:
	case 0xBE:
	case 0xBF:
	{
		auto reg = *RIP & 0x7;
		if (Prefix.B)
			reg += 8;

		if (Prefix.W)
		{
			GPR[reg] = *(UINT64*)(&RIP[1]);
			RIP += 9;
		}
		else if (!Prefix.OperandSize)
		{
			GPR[reg] = (UINT64)*(UINT32*)(&RIP[1]);
			RIP += 5;
		}
		else
		{
			*(UINT32*)&GPR[reg] = (UINT32)*(UINT16*)(&RIP[1]);
			RIP += 3;
		}
		status = true;
	}break;
	case 0xC6:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto imm = *(UINT8*)(&RIP[0]);

			*(BYTE*)ptr = imm;

			RIP++;
			status = true;
		}
		
	}break;
	case 0xC7:
	{
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto imm = *(UINT32*)(&RIP[0]);
			if (Prefix.W)
			{
				*(UINT64*)ptr = (UINT64)imm;
			}
			else if (!Prefix.OperandSize)
			{
				*(UINT64*)ptr = (UINT64)imm;
			}
			else
			{
				*(UINT32*)ptr = (UINT32)(UINT16)imm;
			}
			RIP += 4;
			status = true;
		}
	}break;
	};

	return status;
}