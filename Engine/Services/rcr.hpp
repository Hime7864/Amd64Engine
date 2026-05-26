#pragma once

bool AssemblyState::service_rcr()
{
	bool status = false;

	switch (*RIP)
	{
	case 0xC0:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			UINT8 count = RIP[0];
			auto tempCount = (count & 0x1F) % 9;
			auto countMask = 0x1F;

			if ((count & countMask) == 1)
			{
				FLAGS.OF = get_msb(*(UINT8*)ptr, 8) ^ FLAGS.CF;
			}

			while (tempCount)
			{
				auto tempCF = get_lsb(count, 8);
				*(UINT8*)ptr = (*(UINT8*)ptr >> 1) | (FLAGS.CF << 7);
				FLAGS.CF = tempCF;
				tempCount--;
			}
			
			RIP++;
			status = true;
		}
	}break;
	case 0xC1:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			if (Prefix.W)
			{
				UINT8 count = RIP[0];
				auto tempCount = count & 0x3F;
				auto countMask = 0x3F;

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT64*)ptr, 64) ^ FLAGS.CF;
				}

				while (tempCount)
				{
					auto tempCF = get_lsb(count, 64);
					*(UINT64*)ptr = (*(UINT64*)ptr >> 1) | (FLAGS.CF << 63);
					FLAGS.CF = tempCF;
					tempCount--;
				}
			}
			else if (!Prefix.OperandSize)
			{
				UINT8 count = RIP[0];
				auto tempCount = count & 0x1F;
				auto countMask = 0x1F;

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT32*)ptr, 32) ^ FLAGS.CF;
				}

				while (tempCount)
				{
					auto tempCF = get_lsb(count, 32);
					*(UINT32*)ptr = (*(UINT32*)ptr >> 1) | (FLAGS.CF << 31);
					FLAGS.CF = tempCF;
					tempCount--;
				}
			}
			else
			{
				UINT8 count = RIP[0];
				auto tempCount = (count & 0x1F) % 17;
				auto countMask = 0x1F;

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT16*)ptr, 16) ^ FLAGS.CF;
				}

				while (tempCount)
				{
					auto tempCF = get_lsb(count, 16);
					*(UINT16*)ptr = (*(UINT16*)ptr >> 1) | (FLAGS.CF << 15);
					FLAGS.CF = tempCF;
					tempCount--;
				}
			}
			RIP++;
			status = true;
		}
	}break;
	case 0xD0:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			UINT8 count = 1;
			auto tempCount = (count & 0x1F) % 9;
			auto countMask = 0x1F;

			if ((count & countMask) == 1)
			{
				FLAGS.OF = get_msb(*(UINT8*)ptr, 8) ^ FLAGS.CF;
			}

			while (tempCount)
			{
				auto tempCF = get_lsb(count, 8);
				*(UINT8*)ptr = (*(UINT8*)ptr >> 1) | (FLAGS.CF << 7);
				FLAGS.CF = tempCF;
				tempCount--;
			}

			RIP++;
			status = true;
		}
	}break;
	case 0xD1:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			if (Prefix.W)
			{
				UINT8 count = 1;
				auto tempCount = count & 0x3F;
				auto countMask = 0x3F;

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT64*)ptr, 64) ^ FLAGS.CF;
				}

				while (tempCount)
				{
					auto tempCF = get_lsb(count, 64);
					*(UINT64*)ptr = (*(UINT64*)ptr >> 1) | (FLAGS.CF << 63);
					FLAGS.CF = tempCF;
					tempCount--;
				}
			}
			else if (!Prefix.OperandSize)
			{
				UINT8 count = 1;
				auto tempCount = count & 0x1F;
				auto countMask = 0x1F;

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT32*)ptr, 32) ^ FLAGS.CF;
				}

				while (tempCount)
				{
					auto tempCF = get_lsb(count, 32);
					*(UINT32*)ptr = (*(UINT32*)ptr >> 1) | (FLAGS.CF << 31);
					FLAGS.CF = tempCF;
					tempCount--;
				}
			}
			else
			{
				UINT8 count = 1;
				auto tempCount = (count & 0x1F) % 17;
				auto countMask = 0x1F;

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT16*)ptr, 16) ^ FLAGS.CF;
				}

				while (tempCount)
				{
					auto tempCF = get_lsb(count, 16);
					*(UINT16*)ptr = (*(UINT16*)ptr >> 1) | (FLAGS.CF << 15);
					FLAGS.CF = tempCF;
					tempCount--;
				}
			}
			RIP++;
			status = true;
		}
	}break;
	case 0xD2:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
			auto tempCount = (count & 0x1F) % 9;
			auto countMask = 0x1F;

			if ((count & countMask) == 1)
			{
				FLAGS.OF = get_msb(*(UINT8*)ptr, 8) ^ FLAGS.CF;
			}

			while (tempCount)
			{
				auto tempCF = get_lsb(count, 8);
				*(UINT8*)ptr = (*(UINT8*)ptr >> 1) | (FLAGS.CF << 7);
				FLAGS.CF = tempCF;
				tempCount--;
			}

			RIP++;
			status = true;
		}
	}break;
	case 0xD3:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			if (Prefix.W)
			{
				UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
				auto tempCount = count & 0x3F;
				auto countMask = 0x3F;

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT64*)ptr, 64) ^ FLAGS.CF;
				}

				while (tempCount)
				{
					auto tempCF = get_lsb(count, 64);
					*(UINT64*)ptr = (*(UINT64*)ptr >> 1) | (FLAGS.CF << 63);
					FLAGS.CF = tempCF;
					tempCount--;
				}
			}
			else if (!Prefix.OperandSize)
			{
				UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
				auto tempCount = count & 0x1F;
				auto countMask = 0x1F;

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT32*)ptr, 32) ^ FLAGS.CF;
				}

				while (tempCount)
				{
					auto tempCF = get_lsb(count, 32);
					*(UINT32*)ptr = (*(UINT32*)ptr >> 1) | (FLAGS.CF << 31);
					FLAGS.CF = tempCF;
					tempCount--;
				}
			}
			else
			{
				UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
				auto tempCount = (count & 0x1F) % 17;
				auto countMask = 0x1F;

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT16*)ptr, 16) ^ FLAGS.CF;
				}

				while (tempCount)
				{
					auto tempCF = get_lsb(count, 16);
					*(UINT16*)ptr = (*(UINT16*)ptr >> 1) | (FLAGS.CF << 15);
					FLAGS.CF = tempCF;
					tempCount--;
				}
			}
			RIP++;
			status = true;
		}
	}break;
	}

	return status;
}

