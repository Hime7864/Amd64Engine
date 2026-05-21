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
			switch (modrm->Mode)
			{
			case EMODE::MEM_0_BIT_DISP:
			case EMODE::MEM_8_BIT_DISP:
			case EMODE::MEM_32_BIT_DISP:
			{
				auto ptr = GetDisplacementPtr();
				if (ptr)
				{
					if (state)
					{
						auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
						if (Prefix.W)
						{
							*(UINT64*)&GPR[modrm_register] = *(UINT64*)ptr;
						}
						else
						{
							if (Prefix.OperandSize)
							{
								*(UINT16*)&GPR[modrm_register] = *(UINT16*)ptr;
							}
							else
							{
								GPR[modrm_register] = 0;
								*(UINT32*)&GPR[modrm_register] = *(UINT32*)ptr;
							}
						}
					}
					status = true;
				}
			}break;
			case EMODE::REG_TO_REG:
			{
				if (state)
				{
					auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
					auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
					if (Prefix.W)
					{
						*(UINT64*)&GPR[modrm_register] = *(UINT64*)&GPR[modrm_register_memory];
					}
					else
					{
						if (Prefix.OperandSize)
						{
							*(UINT16*)&GPR[modrm_register] = *(UINT16*)&GPR[modrm_register_memory];
						}
						else
						{
							GPR[modrm_register] = 0;
							*(UINT32*)&GPR[modrm_register] = *(UINT32*)&GPR[modrm_register_memory];
						}
					}
				}
				RIP += 2;
				status = true;
			}break;
			}
		}
		else
		{
			switch (*RIP)
			{
			case 0x10:
			{
				auto modrm = (MODRM*)(&RIP[2]);
				switch (modrm->Mode)
				{
				case EMODE::MEM_0_BIT_DISP:
				case EMODE::MEM_8_BIT_DISP:
				case EMODE::MEM_32_BIT_DISP:
				{
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
				case EMODE::REG_TO_REG:
				{
					auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
					auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

					if (Prefix.OperandSize)
					{
						XMM[modrm_register] = XMM[modrm_register_memory];
					}
					else if (Prefix.RepeatedNE)
					{
						XMM[modrm_register].u64.data[0] = XMM[modrm_register_memory].u64.data[0];
					}
					else if (Prefix.Repeated)
					{
						XMM[modrm_register].u32.data[0] = XMM[modrm_register_memory].u32.data[0];
					}
					else
					{
						XMM[modrm_register] = XMM[modrm_register_memory];
					}

					RIP += 2;
					status = true;
				}break;
				};
			}break;
			case 0x11:
			{
				auto modrm = (MODRM*)(&RIP[2]);
				switch (modrm->Mode)
				{
				case EMODE::MEM_0_BIT_DISP:
				case EMODE::MEM_8_BIT_DISP:
				case EMODE::MEM_32_BIT_DISP:
				{
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
				case EMODE::REG_TO_REG:
				{
					auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
					auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

					if (Prefix.OperandSize)
					{
						XMM[modrm_register_memory] = XMM[modrm_register];
					}
					else if (Prefix.RepeatedNE)
					{
						XMM[modrm_register_memory].u64.data[0] = XMM[modrm_register].u64.data[0];
					}
					else if (Prefix.Repeated)
					{
						XMM[modrm_register_memory].u32.data[0] = XMM[modrm_register].u32.data[0];
					}
					else
					{
						XMM[modrm_register_memory] = XMM[modrm_register];
					}

					RIP += 2;
					status = true;
				}break;
				};
			}break;
			case 0xB6:
			{
				auto modrm = (MODRM*)&RIP[1];
				switch (modrm->Mode)
				{
				case EMODE::MEM_0_BIT_DISP:
				case EMODE::MEM_8_BIT_DISP:
				case EMODE::MEM_32_BIT_DISP:
				{
					auto ptr = GetDisplacementPtr();
					if (ptr)
					{
						auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
						if (Prefix.W)
						{
							GPR[modrm_register] = ((UINT64) * (BYTE*)ptr) & 0xFFull;
						}
						else
						{
							if (Prefix.OperandSize)
							{
								*(UINT16*)&GPR[modrm_register] = ((UINT64) * (BYTE*)ptr) & 0xFFull;
							}
							else
							{
								GPR[modrm_register] = 0;
								*(UINT32*)&GPR[modrm_register] = ((UINT64) * (BYTE*)ptr) & 0xFFull;
							}
						}
						status = true;
					}
				}break;
				case EMODE::REG_TO_REG:
				{
					auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
					auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
					if (Prefix.W)
					{
						GPR[modrm_register] = ((UINT64) * (BYTE*)&GPR[modrm_register_memory]) & 0xFFull;
					}
					else
					{
						if (Prefix.OperandSize)
						{
							*(UINT16*)&GPR[modrm_register] = ((UINT64) * (BYTE*)&GPR[modrm_register_memory]) & 0xFFull;
						}
						else
						{
							GPR[modrm_register] = 0;
							*(UINT32*)&GPR[modrm_register] = ((UINT64) * (BYTE*)&GPR[modrm_register_memory]) & 0xFFull;
						}
					}
					RIP += 2;
					status = true;
				}break;
				}
			}break;
			case 0xB7:
			{
				auto modrm = (MODRM*)&RIP[1];
				switch (modrm->Mode)
				{
				case EMODE::MEM_0_BIT_DISP:
				case EMODE::MEM_8_BIT_DISP:
				case EMODE::MEM_32_BIT_DISP:
				{
					auto ptr = GetDisplacementPtr();
					if (ptr)
					{
						auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
						if (Prefix.W)
						{
							GPR[modrm_register] = ((UINT64) * (UINT16*)ptr) & 0xFFFFull;
						}
						else
						{
							if (Prefix.OperandSize)
							{
								*(UINT16*)&GPR[modrm_register] = ((UINT64) * (UINT16*)ptr) & 0xFFFFull;
							}
							else
							{
								GPR[modrm_register] = 0;
								*(UINT32*)&GPR[modrm_register] = ((UINT64) * (UINT16*)ptr) & 0xFFFFull;
							}
						}
						status = true;
					}
				}break;
				case EMODE::REG_TO_REG:
				{
					auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
					auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
					if (Prefix.W)
					{
						GPR[modrm_register] = ((UINT64) * (UINT16*)&GPR[modrm_register_memory]) & 0xFFFFull;
					}
					else
					{
						if (Prefix.OperandSize)
						{
							*(UINT16*)&GPR[modrm_register] = ((UINT64) * (UINT16*)&GPR[modrm_register_memory]) & 0xFFFFull;
						}
						else
						{
							GPR[modrm_register] = 0;
							*(UINT32*)&GPR[modrm_register] = ((UINT64) * (UINT16*)&GPR[modrm_register_memory]) & 0xFFFFull;
						}
					}
					RIP += 2;
					status = true;
				}break;
				}
			}break;
			}
		}
	}break;
	case 0x88:
	{
		auto modrm = (MODRM*)(&RIP[1]);

		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				*(BYTE*)ptr = *(BYTE*)&GPR[modrm_register];
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			*(BYTE*)&GPR[modrm_register_memory] = *(BYTE*)&GPR[modrm_register];

			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x89:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)
				{
					*(UINT64*)ptr = GPR[modrm_register];
				}
				else
				{
					if (Prefix.OperandSize)
					{
						*(UINT16*)ptr = *(UINT16*)&GPR[modrm_register];
					}
					else
					{
						*(UINT32*)ptr = *(UINT32*)&GPR[modrm_register];
					}
				}
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			if (Prefix.W)
			{
				GPR[modrm_register_memory] = GPR[modrm_register];
			}
			else
			{
				GPR[modrm_register_memory] = 0;
				*(UINT32*)&GPR[modrm_register_memory] = *(UINT32*)&GPR[modrm_register];
			}
			RIP += 2;
			status = true;
		}break;
		};

	}break;
	case 0x8A:
	{
		auto modrm = (MODRM*)(&RIP[1]);

		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				*(BYTE*)&GPR[modrm_register] = *(BYTE*)ptr;
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			*(BYTE*)&GPR[modrm_register] = *(BYTE*)&GPR[modrm_register_memory];

			RIP += 2;
			status = true;
		}break;
		};
	}break;
	case 0x8B:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)
				{
					GPR[modrm_register] = *(UINT64*)ptr;
				}
				else
				{
					if (Prefix.OperandSize)
					{
						*(UINT16*)&GPR[modrm_register] = *(UINT16*)ptr;
					}
					else
					{
						GPR[modrm_register] = 0;
						*(UINT32*)&GPR[modrm_register] = *(UINT32*)ptr;
					}
				}
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
			if (Prefix.W)
			{
				GPR[modrm_register] = GPR[modrm_register_memory];
			}
			else
			{
				if (Prefix.OperandSize)
				{
					*(UINT16*)&GPR[modrm_register] = *(UINT16*)&GPR[modrm_register_memory];
				}
				else
				{
					GPR[modrm_register] = 0;
					*(UINT32*)&GPR[modrm_register] = *(UINT32*)&GPR[modrm_register_memory];
				}
			}
			RIP += 2;
			status = true;
		}break;
		};
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
	case 0xA0:
	{
		auto moffs = *(UINT64*)(&RIP[1]);
		*(BYTE*)&GPR[(BYTE)EGPR::RAX] = (BYTE)moffs;
		RIP += 9;
		status = true;
	}break;
	case 0xA1:
	{
		auto moffs = *(UINT64*)(&RIP[1]);
		if (Prefix.W)
		{
			GPR[(BYTE)EGPR::RAX] = moffs;
		}
		else
		{
			if (Prefix.OperandSize)
			{
				*(UINT16*)&GPR[(BYTE)EGPR::RAX] = (UINT16)moffs;
			}
			else
			{
				GPR[(BYTE)EGPR::RAX] = 0;
				*(UINT32*)&GPR[(BYTE)EGPR::RAX] = (UINT32)moffs;
			}
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
		else
		{
			if (Prefix.OperandSize)
			{
				*(UINT16*)moffs = *(UINT16*)&GPR[(BYTE)EGPR::RAX];
			}
			else
			{
				*(UINT32*)moffs = *(UINT32*)&GPR[(BYTE)EGPR::RAX];
			}
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
		if (Prefix.B)
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
		else
		{
			if (Prefix.OperandSize)
			{
				*(WORD*)&GPR[reg] = *(WORD*)(&RIP[1]);
				RIP += 3;
			}
			else
			{
				GPR[reg] = *(UINT32*)(&RIP[1]);
				RIP += 5;

			}
		}
		status = true;
	}break;
	case 0xC6:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto imm = *(UINT8*)(&RIP[2]);
			auto ptr = GetDisplacementPtr();

			*(BYTE*)ptr = imm;

			RIP++;
			status = true;
		}break;
		case EMODE::REG_TO_REG:
		{
			auto imm = *(UINT8*)(&RIP[2]);
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;

			*(BYTE*)&GPR[modrm_register_memory] = imm;

			RIP += 3;
			status = true;
		}break;
		};
	}break;
	case 0xC7:
	{
		auto modrm = (MODRM*)(&RIP[1]);

		switch (modrm->Mode)
		{
		case EMODE::MEM_0_BIT_DISP:
		case EMODE::MEM_8_BIT_DISP:
		case EMODE::MEM_32_BIT_DISP:
		{
			auto ptr = GetDisplacementPtr();
			auto imm = 0ll + *(INT32*)RIP;
			if (ptr)
			{
				if (Prefix.W)
				{
					*(UINT64*)ptr = (UINT64)imm;
				}
				else
				{
					if (Prefix.OperandSize)
					{
						*(UINT16*)ptr = (UINT16)imm;
					}
					else
					{
						*(UINT32*)ptr = (UINT32)imm;
					}
				}
				RIP += 4;
				status = true;
			}
		}break;
		case EMODE::REG_TO_REG:
		{
			auto imm = *(UINT32*)(&RIP[2]);
			auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
			if (Prefix.W)
			{
				GPR[modrm_register_memory] = imm;
			}
			else
			{
				if (Prefix.OperandSize)
				{
					*(UINT16*)&GPR[modrm_register_memory] = (UINT16)imm;
				}
				else
				{
					GPR[modrm_register_memory] = 0;
					*(UINT32*)&GPR[modrm_register_memory] = imm;
				}
			}
			RIP += 6;
			status = true;
		}break;
		};
	}break;
	};

	if (status)
		printf("MOVE");

	return status;
}