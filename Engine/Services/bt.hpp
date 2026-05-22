#pragma once

bool AssemblyState::service_bt()
{
	bool status = false;

	switch (*RIP)
	{
	case 0x0F:
	{
		RIP++;
		switch (*RIP)
		{
		case 0xA3:
		{
			// r/m16/32/64	r16/32/64
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
						auto bitBase = *(UINT64*)ptr;
						auto bitOffset = *(UINT64*)&GPR[modrm_register];
						auto bitIndex = bitOffset & 0x3F;
						auto bitMask = 1ULL << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
					}
					else
					{
						if (Prefix.OperandSize)
						{
							auto bitBase = *(UINT16*)ptr;
							auto bitOffset = *(UINT16*)&GPR[modrm_register];
							auto bitIndex = bitOffset & 0x0F;
							auto bitMask = 1U << bitIndex;
							FLAGS.CF = (bitBase & bitMask) != 0;
						}
						else
						{
							auto bitBase = *(UINT32*)ptr;
							auto bitOffset = *(UINT32*)&GPR[modrm_register];
							auto bitIndex = bitOffset & 0x1F;
							auto bitMask = 1U << bitIndex;
							FLAGS.CF = (bitBase & bitMask) != 0;
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
					auto bitBase = *(UINT64*)&GPR[modrm_register_memory];
					auto bitOffset = *(UINT64*)&GPR[modrm_register];
					auto bitIndex = bitOffset & 0x3F;
					auto bitMask = 1ULL << bitIndex;
					FLAGS.CF = (bitBase & bitMask) != 0;
				}
				else
				{
					if (Prefix.OperandSize)
					{
						auto bitBase = *(UINT16*)&GPR[modrm_register_memory];
						auto bitOffset = *(UINT16*)&GPR[modrm_register];
						auto bitIndex = bitOffset & 0x0F;
						auto bitMask = 1U << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
					}
					else
					{
						auto bitBase = *(UINT32*)&GPR[modrm_register_memory];
						auto bitOffset = *(UINT32*)&GPR[modrm_register];
						auto bitIndex = bitOffset & 0x1F;
						auto bitMask = 1U << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
					}
				}
				RIP++;
				status = true;
			}
			};
		}break;
		case 0xAB:
		{
			// r/m16/32/64	r16/32/64
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
						auto bitBase = *(UINT64*)ptr;
						auto bitOffset = *(UINT64*)&GPR[modrm_register];
						auto bitIndex = bitOffset & 0x3F;
						auto bitMask = 1ULL << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
						bitBase |= bitMask;
						*(UINT64*)ptr = bitBase;
					}
					else
					{
						if (Prefix.OperandSize)
						{
							auto bitBase = *(UINT16*)ptr;
							auto bitOffset = *(UINT16*)&GPR[modrm_register];
							auto bitIndex = bitOffset & 0x0F;
							auto bitMask = 1U << bitIndex;
							FLAGS.CF = (bitBase & bitMask) != 0;
							bitBase |= bitMask;
							*(UINT64*)ptr = bitBase;
						}
						else
						{
							auto bitBase = *(UINT32*)ptr;
							auto bitOffset = *(UINT32*)&GPR[modrm_register];
							auto bitIndex = bitOffset & 0x1F;
							auto bitMask = 1U << bitIndex;
							FLAGS.CF = (bitBase & bitMask) != 0;
							bitBase |= bitMask;
							*(UINT64*)ptr = bitBase;
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
					auto bitBase = *(UINT64*)&GPR[modrm_register_memory];
					auto bitOffset = *(UINT64*)&GPR[modrm_register];
					auto bitIndex = bitOffset & 0x3F;
					auto bitMask = 1ULL << bitIndex;
					FLAGS.CF = (bitBase & bitMask) != 0;
					bitBase |= bitMask;
					*(UINT64*)&GPR[modrm_register_memory] = bitBase;
				}
				else
				{
					if (Prefix.OperandSize)
					{
						auto bitBase = *(UINT16*)&GPR[modrm_register_memory];
						auto bitOffset = *(UINT16*)&GPR[modrm_register];
						auto bitIndex = bitOffset & 0x0F;
						auto bitMask = 1U << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
						bitBase |= bitMask;
						*(UINT16*)&GPR[modrm_register_memory] = bitBase;
					}
					else
					{
						auto bitBase = *(UINT32*)&GPR[modrm_register_memory];
						auto bitOffset = *(UINT32*)&GPR[modrm_register];
						auto bitIndex = bitOffset & 0x1F;
						auto bitMask = 1U << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
						bitBase |= bitMask;
						*(UINT32*)&GPR[modrm_register_memory] = bitBase;
					}
				}
				RIP++;
				status = true;
			}
			};
		}break;
		case 0xB3:
		{
			// r/m16/32/64	r16/32/64
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
						auto bitBase = *(UINT64*)ptr;
						auto bitOffset = *(UINT64*)&GPR[modrm_register];
						auto bitIndex = bitOffset & 0x3F;
						auto bitMask = 1ULL << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
						bitBase &= ~bitMask;
						*(UINT64*)ptr = bitBase;
					}
					else
					{
						if (Prefix.OperandSize)
						{
							auto bitBase = *(UINT16*)ptr;
							auto bitOffset = *(UINT16*)&GPR[modrm_register];
							auto bitIndex = bitOffset & 0x0F;
							auto bitMask = 1U << bitIndex;
							FLAGS.CF = (bitBase & bitMask) != 0;
							bitBase &= ~bitMask;
							*(UINT16*)ptr = bitBase;
						}
						else
						{
							auto bitBase = *(UINT32*)ptr;
							auto bitOffset = *(UINT32*)&GPR[modrm_register];
							auto bitIndex = bitOffset & 0x1F;
							auto bitMask = 1U << bitIndex;
							FLAGS.CF = (bitBase & bitMask) != 0;
							bitBase &= ~bitMask;
							*(UINT32*)ptr = bitBase;
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
					auto bitBase = *(UINT64*)&GPR[modrm_register_memory];
					auto bitOffset = *(UINT64*)&GPR[modrm_register];
					auto bitIndex = bitOffset & 0x3F;
					auto bitMask = 1ULL << bitIndex;
					FLAGS.CF = (bitBase & bitMask) != 0;
					bitBase &= ~bitMask;
					*(UINT64*)&GPR[modrm_register_memory] = bitBase;
				}
				else
				{
					if (Prefix.OperandSize)
					{
						auto bitBase = *(UINT16*)&GPR[modrm_register_memory];
						auto bitOffset = *(UINT16*)&GPR[modrm_register];
						auto bitIndex = bitOffset & 0x0F;
						auto bitMask = 1U << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
						bitBase &= ~bitMask;
						*(UINT16*)&GPR[modrm_register_memory] = bitBase;
					}
					else
					{
						auto bitBase = *(UINT32*)&GPR[modrm_register_memory];
						auto bitOffset = *(UINT32*)&GPR[modrm_register];
						auto bitIndex = bitOffset & 0x1F;
						auto bitMask = 1U << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
						bitBase &= ~bitMask;
						*(UINT32*)&GPR[modrm_register_memory] = bitBase;
					}
				}
				RIP++;
				status = true;
			}
			};
		}break;
		case 0xBA:
		{
			// r/m16/32/64	imm8
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
					auto imm = *(UINT8*)(&RIP[1]);
					if (Prefix.W)
					{
						auto bitBase = *(UINT64*)ptr;
						auto bitIndex = imm & 0x3F;
						auto bitMask = 1ULL << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
						switch (modrm->Register)
						{
						case 5:
						{
							bitBase |= bitMask;
							*(UINT64*)ptr = bitBase;
						}break;
						case 6:
						{
							bitBase &= ~bitMask;
							*(UINT64*)ptr = bitBase;
						}break;
						case 7:
						{
							bitBase ^= bitMask;
							*(UINT64*)ptr = bitBase;
						}break;
						};
					}
					else
					{
						if (Prefix.OperandSize)
						{
							auto bitBase = *(UINT16*)ptr;
							auto bitIndex = imm & 0x0F;
							auto bitMask = 1U << bitIndex;
							FLAGS.CF = (bitBase & bitMask) != 0;
							switch (modrm->Register)
							{
							case 5:
							{
								bitBase |= bitMask;
								*(UINT16*)ptr = bitBase;
							}break;
							case 6:
							{
								bitBase &= ~bitMask;
								*(UINT16*)ptr = bitBase;
							}break;
							case 7:
							{
								bitBase ^= bitMask;
								*(UINT16*)ptr = bitBase;
							}break;
							};
						}
						else
						{
							auto bitBase = *(UINT32*)ptr;
							auto bitIndex = imm & 0x1F;
							auto bitMask = 1U << bitIndex;
							FLAGS.CF = (bitBase & bitMask) != 0;
							switch (modrm->Register)
							{
							case 5:
							{
								bitBase |= bitMask;
								*(UINT32*)ptr = bitBase;
							}break;
							case 6:
							{
								bitBase &= ~bitMask;
								*(UINT32*)ptr = bitBase;
							}break;
							case 7:
							{
								bitBase ^= bitMask;
								*(UINT32*)ptr = bitBase;
							}break;
							};
						}
					}
					RIP++;
					status = true;
				}
			}break;
			case EMODE::REG_TO_REG:
			{
				auto imm = *(UINT8*)(&RIP[2]);
				auto modrm_register_memory = Prefix.B ? modrm->RegisterMemory + 8 : modrm->RegisterMemory;
				if (Prefix.W)
				{
					auto bitBase = *(UINT64*)&GPR[modrm_register_memory];
					auto bitIndex = imm & 0x3F;
					auto bitMask = 1ULL << bitIndex;
					FLAGS.CF = (bitBase & bitMask) != 0;
					switch (modrm->Register)
					{
					case 5:
					{
						bitBase |= bitMask;
						*(UINT64*)&GPR[modrm_register_memory] = bitBase;
					}break;
					case 6:
					{
						bitBase &= ~bitMask;
						*(UINT64*)&GPR[modrm_register_memory] = bitBase;
					}break;
					case 7:
					{
						bitBase ^= bitMask;
						*(UINT64*)&GPR[modrm_register_memory] = bitBase;
					}break;
					};
				}
				else
				{
					if (Prefix.OperandSize)
					{
						auto bitBase = *(UINT16*)&GPR[modrm_register_memory];
						auto bitIndex = imm & 0x0F;
						auto bitMask = 1U << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
						switch (modrm->Register)
						{
						case 5:
						{
							bitBase |= bitMask;
							*(UINT16*)&GPR[modrm_register_memory] = bitBase;
						}break;
						case 6:
						{
							bitBase &= ~bitMask;
							*(UINT16*)&GPR[modrm_register_memory] = bitBase;
						}break;
						case 7:
						{
							bitBase ^= bitMask;
							*(UINT16*)&GPR[modrm_register_memory] = bitBase;
						}break;
						};
					}
					else
					{
						auto bitBase = *(UINT32*)&GPR[modrm_register_memory];
						auto bitIndex = imm & 0x1F;
						auto bitMask = 1U << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
						switch (modrm->Register)
						{
						case 5:
						{
							bitBase |= bitMask;
							*(UINT32*)&GPR[modrm_register_memory] = bitBase;
						}break;
						case 6:
						{
							bitBase &= ~bitMask;
							*(UINT32*)&GPR[modrm_register_memory] = bitBase;
						}break;
						case 7:
						{
							bitBase ^= bitMask;
							*(UINT32*)&GPR[modrm_register_memory] = bitBase;
						}break;
						};
					}
				}
				RIP+=3;
				status = true;
			}
			};
		}break;
		case 0xBB:
		{
			// r/m16/32/64	r16/32/64
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
						auto bitBase = *(UINT64*)ptr;
						auto bitOffset = *(UINT64*)&GPR[modrm_register];
						auto bitIndex = bitOffset & 0x3F;
						auto bitMask = 1ULL << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
						bitBase ^= bitMask;
						*(UINT64*)ptr = bitBase;
					}
					else
					{
						if (Prefix.OperandSize)
						{
							auto bitBase = *(UINT16*)ptr;
							auto bitOffset = *(UINT16*)&GPR[modrm_register];
							auto bitIndex = bitOffset & 0x0F;
							auto bitMask = 1U << bitIndex;
							FLAGS.CF = (bitBase & bitMask) != 0;
							bitBase ^= bitMask;
							*(UINT16*)ptr = bitBase;
						}
						else
						{
							auto bitBase = *(UINT32*)ptr;
							auto bitOffset = *(UINT32*)&GPR[modrm_register];
							auto bitIndex = bitOffset & 0x1F;
							auto bitMask = 1U << bitIndex;
							FLAGS.CF = (bitBase & bitMask) != 0;
							bitBase ^= bitMask;
							*(UINT32*)ptr = bitBase;
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
					auto bitBase = *(UINT64*)&GPR[modrm_register_memory];
					auto bitOffset = *(UINT64*)&GPR[modrm_register];
					auto bitIndex = bitOffset & 0x3F;
					auto bitMask = 1ULL << bitIndex;
					FLAGS.CF = (bitBase & bitMask) != 0;
					bitBase ^= bitMask;
					*(UINT64*)&GPR[modrm_register_memory] = bitBase;
				}
				else
				{
					if (Prefix.OperandSize)
					{
						auto bitBase = *(UINT16*)&GPR[modrm_register_memory];
						auto bitOffset = *(UINT16*)&GPR[modrm_register];
						auto bitIndex = bitOffset & 0x0F;
						auto bitMask = 1U << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
						bitBase ^= bitMask;
						*(UINT16*)&GPR[modrm_register_memory] = bitBase;
					}
					else
					{
						auto bitBase = *(UINT32*)&GPR[modrm_register_memory];
						auto bitOffset = *(UINT32*)&GPR[modrm_register];
						auto bitIndex = bitOffset & 0x1F;
						auto bitMask = 1U << bitIndex;
						FLAGS.CF = (bitBase & bitMask) != 0;
						bitBase ^= bitMask;
						*(UINT32*)&GPR[modrm_register_memory] = bitBase;
					}
				}
				RIP++;
				status = true;
			}
			};
		}break;
		};
	}break;
	};

	if (status)
		printf("Bit Test");

	return status;
}

