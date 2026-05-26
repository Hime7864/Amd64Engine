#pragma once

bool AssemblyState::service_sal()
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
			auto tempCount = count & 0x1F;
			auto countMask = 0x1F;

			while (tempCount)
			{
				FLAGS.CF = get_msb(*(UINT8*)ptr, 8);
				*(UINT8*)ptr = (*(UINT8*)ptr * 2);
				tempCount--;
			}

			if ((count & countMask) == 1)
			{
				FLAGS.OF = get_msb(*(UINT8*)ptr, 8) ^ FLAGS.CF;
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

				while (tempCount)
				{
					FLAGS.CF = get_msb(*(UINT64*)ptr, 64);
					*(UINT64*)ptr = (*(UINT64*)ptr * 2);
					tempCount--;
				}

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT64*)ptr, 64) ^ FLAGS.CF;
				}
			}
			else if (!Prefix.OperandSize)
			{
				UINT8 count = RIP[0];
				auto tempCount = count & 0x1F;
				auto countMask = 0x1F;

				while (tempCount)
				{
					FLAGS.CF = get_msb(*(UINT32*)ptr, 32);
					*(UINT32*)ptr = (*(UINT32*)ptr * 2);
					tempCount--;
				}

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT32*)ptr, 32) ^ FLAGS.CF;
				}
			}
			else
			{
				UINT8 count = RIP[0];
				auto tempCount = count & 0x1F;
				auto countMask = 0x1F;

				while (tempCount)
				{
					FLAGS.CF = get_msb(*(UINT16*)ptr, 16);
					*(UINT16*)ptr = (*(UINT16*)ptr * 2);
					tempCount--;
				}

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT16*)ptr, 16) ^ FLAGS.CF;
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
			auto tempCount = count & 0x1F;
			auto countMask = 0x1F;

			while (tempCount)
			{
				FLAGS.CF = get_msb(*(UINT8*)ptr, 8);
				*(UINT8*)ptr = (*(UINT8*)ptr * 2);
				tempCount--;
			}

			if ((count & countMask) == 1)
			{
				FLAGS.OF = get_msb(*(UINT8*)ptr, 8) ^ FLAGS.CF;
			}
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

				while (tempCount)
				{
					FLAGS.CF = get_msb(*(UINT64*)ptr, 64);
					*(UINT64*)ptr = (*(UINT64*)ptr * 2);
					tempCount--;
				}

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT64*)ptr, 64) ^ FLAGS.CF;
				}
			}
			else if (!Prefix.OperandSize)
			{
				UINT8 count = 1;
				auto tempCount = count & 0x1F;
				auto countMask = 0x1F;

				while (tempCount)
				{
					FLAGS.CF = get_msb(*(UINT32*)ptr, 32);
					*(UINT32*)ptr = (*(UINT32*)ptr * 2);
					tempCount--;
				}

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT32*)ptr, 32) ^ FLAGS.CF;
				}
			}
			else
			{
				UINT8 count = 1;
				auto tempCount = count & 0x1F;
				auto countMask = 0x1F;

				while (tempCount)
				{
					FLAGS.CF = get_msb(*(UINT16*)ptr, 16);
					*(UINT16*)ptr = (*(UINT16*)ptr * 2);
					tempCount--;
				}

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT16*)ptr, 16) ^ FLAGS.CF;
				}
			}
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
			auto tempCount = count & 0x1F;
			auto countMask = 0x1F;

			while (tempCount)
			{
				FLAGS.CF = get_msb(*(UINT8*)ptr, 8);
				*(UINT8*)ptr = (*(UINT8*)ptr * 2);
				tempCount--;
			}

			if ((count & countMask) == 1)
			{
				FLAGS.OF = get_msb(*(UINT8*)ptr, 8) ^ FLAGS.CF;
			}
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

				while (tempCount)
				{
					FLAGS.CF = get_msb(*(UINT64*)ptr, 64);
					*(UINT64*)ptr = (*(UINT64*)ptr * 2);
					tempCount--;
				}

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT64*)ptr, 64) ^ FLAGS.CF;
				}
			}
			else if (!Prefix.OperandSize)
			{
				UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
				auto tempCount = count & 0x1F;
				auto countMask = 0x1F;

				while (tempCount)
				{
					FLAGS.CF = get_msb(*(UINT32*)ptr, 32);
					*(UINT32*)ptr = (*(UINT32*)ptr * 2);
					tempCount--;
				}

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT32*)ptr, 32) ^ FLAGS.CF;
				}
			}
			else
			{
				UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
				auto tempCount = count & 0x1F;
				auto countMask = 0x1F;

				while (tempCount)
				{
					FLAGS.CF = get_msb(*(UINT16*)ptr, 16);
					*(UINT16*)ptr = (*(UINT16*)ptr * 2);
					tempCount--;
				}

				if ((count & countMask) == 1)
				{
					FLAGS.OF = get_msb(*(UINT16*)ptr, 16) ^ FLAGS.CF;
				}
			}
			status = true;
		}
	}break;
	}

	return status;
}

