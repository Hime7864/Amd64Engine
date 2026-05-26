#pragma once

bool AssemblyState::service_sar()
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

			if ((count & countMask) == 1)
			{
				FLAGS.OF = 0;
			}

			while (tempCount)
			{
				FLAGS.CF = get_lsb(*(UINT8*)ptr, 8);
				*(UINT8*)ptr = (*(INT8*)ptr / 2);
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
					FLAGS.OF = 0;
				}

				while (tempCount)
				{
					FLAGS.CF = get_lsb(*(UINT64*)ptr, 64);
					*(UINT64*)ptr = (*(INT64*)ptr / 2);
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
					FLAGS.OF = 0;
				}

				while (tempCount)
				{
					FLAGS.CF = get_lsb(*(UINT32*)ptr, 32);
					*(UINT32*)ptr = (*(INT32*)ptr / 2);
					tempCount--;
				}
			}
			else
			{
				UINT8 count = RIP[0];
				auto tempCount = count & 0x1F;
				auto countMask = 0x1F;

				if ((count & countMask) == 1)
				{
					FLAGS.OF = 0;
				}

				while (tempCount)
				{
					FLAGS.CF = get_lsb(*(UINT16*)ptr, 16);
					*(UINT16*)ptr = (*(INT16*)ptr / 2);
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
			auto tempCount = count & 0x1F;
			auto countMask = 0x1F;

			if ((count & countMask) == 1)
			{
				FLAGS.OF = 0;
			}

			while (tempCount)
			{
				FLAGS.CF = get_lsb(*(UINT8*)ptr, 8);
				*(UINT8*)ptr = (*(INT8*)ptr / 2);
				tempCount--;
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

				if ((count & countMask) == 1)
				{
					FLAGS.OF = 0;
				}

				while (tempCount)
				{
					FLAGS.CF = get_lsb(*(UINT64*)ptr, 64);
					*(UINT64*)ptr = (*(INT64*)ptr / 2);
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
					FLAGS.OF = 0;
				}

				while (tempCount)
				{
					FLAGS.CF = get_lsb(*(UINT32*)ptr, 32);
					*(UINT32*)ptr = (*(INT32*)ptr / 2);
					tempCount--;
				}
			}
			else
			{
				UINT8 count = 1;
				auto tempCount = count & 0x1F;
				auto countMask = 0x1F;

				if ((count & countMask) == 1)
				{
					FLAGS.OF = 0;
				}

				while (tempCount)
				{
					FLAGS.CF = get_lsb(*(UINT16*)ptr, 16);
					*(UINT16*)ptr = (*(INT16*)ptr / 2);
					tempCount--;
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

			if ((count & countMask) == 1)
			{
				FLAGS.OF = 0;
			}

			while (tempCount)
			{
				FLAGS.CF = get_lsb(*(UINT8*)ptr, 8);
				*(UINT8*)ptr = (*(INT8*)ptr / 2);
				tempCount--;
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

				if ((count & countMask) == 1)
				{
					FLAGS.OF = 0;
				}

				while (tempCount)
				{
					FLAGS.CF = get_lsb(*(UINT64*)ptr, 64);
					*(UINT64*)ptr = (*(INT64*)ptr / 2);
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
					FLAGS.OF = 0;
				}

				while (tempCount)
				{
					FLAGS.CF = get_lsb(*(UINT32*)ptr, 32);
					*(UINT32*)ptr = (*(INT32*)ptr / 2);
					tempCount--;
				}
			}
			else
			{
				UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
				auto tempCount = count & 0x1F;
				auto countMask = 0x1F;

				if ((count & countMask) == 1)
				{
					FLAGS.OF = 0;
				}

				while (tempCount)
				{
					FLAGS.CF = get_lsb(*(UINT16*)ptr, 16);
					*(UINT16*)ptr = (*(INT16*)ptr / 2);
					tempCount--;
				}
			}
			status = true;
		}
	}break;
	}

	return status;
}

