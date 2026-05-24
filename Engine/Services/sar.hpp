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
			auto dest = *(UINT8*)ptr;
			UINT8 count = RIP[0];
			UINT8 masked = count & 7;

			if (masked)
			{
				auto result = (UINT8)((INT8)dest >> masked);
				*(UINT8*)ptr = result;

				FLAGS.CF = (dest >> (masked - 1)) & 1;
				if (masked == 1)
					FLAGS.OF = (dest >> 7) & 1;
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
				auto dest = *(UINT64*)ptr;
				UINT8 count = RIP[0];
				UINT8 masked = count & 63;

				if (masked)
				{
					auto result = (UINT64)((INT64)dest >> masked);
					*(UINT64*)ptr = result;

					FLAGS.CF = (dest >> (masked - 1)) & 1;
					if (masked == 1)
						FLAGS.OF = (dest >> 63) & 1;
				}
			}
			else if (!Prefix.OperandSize)
			{
				auto dest = *(UINT32*)ptr;
				UINT8 count = RIP[0];
				UINT8 masked = count & 31;

				if (masked)
				{
					auto result = (UINT32)((INT32)dest >> masked);
					*(UINT32*)ptr = result;

					FLAGS.CF = (dest >> (masked - 1)) & 1;
					if (masked == 1)
						FLAGS.OF = (dest >> 31) & 1;
				}
			}
			else
			{
				auto dest = *(UINT16*)ptr;
				UINT8 count = RIP[0];
				UINT8 masked = count & 15;

				if (masked)
				{
					auto result = (UINT16)((INT16)dest >> masked);
					*(UINT16*)ptr = result;

					FLAGS.CF = (dest >> (masked - 1)) & 1;
					if (masked == 1)
						FLAGS.OF = (dest >> 15) & 1;
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
			auto dest = *(UINT8*)ptr;
			UINT8 count = 1;
			UINT8 masked = count & 7;

			if (masked)
			{
				auto result = (UINT8)((INT8)dest >> masked);
				*(UINT8*)ptr = result;

				FLAGS.CF = (dest >> (masked - 1)) & 1;
				if (masked == 1)
					FLAGS.OF = (dest >> 7) & 1;
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
				auto dest = *(UINT64*)ptr;
				UINT8 count = 1;
				UINT8 masked = count & 63;

				if (masked)
				{
					auto result = (UINT64)((INT64)dest >> masked);
					*(UINT64*)ptr = result;
					FLAGS.CF = (dest >> (masked - 1)) & 1;
					if (masked == 1)
						FLAGS.OF = (dest >> 63) & 1;
				}
			}
			else if (!Prefix.OperandSize)
			{
				auto dest = *(UINT32*)ptr;
				UINT8 count = 1;
				UINT8 masked = count & 31;

				if (masked)
				{
					auto result = (UINT32)((INT32)dest >> masked);
					*(UINT32*)ptr = result;
					FLAGS.CF = (dest >> (masked - 1)) & 1;
					if (masked == 1)
						FLAGS.OF = (dest >> 31) & 1;
				}
			}
			else
			{
				auto dest = *(UINT16*)ptr;
				UINT8 count = 1;
				UINT8 masked = count & 15;

				if (masked)
				{
					auto result = (UINT16)((INT16)dest >> masked);
					*(UINT16*)ptr = result;
					FLAGS.CF = (dest >> (masked - 1)) & 1;
					if (masked == 1)
						FLAGS.OF = (dest >> 15) & 1;
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
			auto dest = *(UINT8*)ptr;
			UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
			UINT8 masked = count & 7;

			if (masked)
			{
				auto result = (UINT8)((INT8)dest >> masked);
				*(UINT8*)ptr = result;
				FLAGS.CF = (dest >> (masked - 1)) & 1;
				if (masked == 1)
					FLAGS.OF = (dest >> 7) & 1;
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
				auto dest = *(UINT64*)ptr;
				UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
				UINT8 masked = count & 63;

				if (masked)
				{
					auto result = (UINT64)((INT64)dest >> masked);
					*(UINT64*)ptr = result;
					FLAGS.CF = (dest >> (masked - 1)) & 1;
					if (masked == 1)
						FLAGS.OF = (dest >> 63) & 1;
				}
			}
			else if (!Prefix.OperandSize)
			{
				auto dest = *(UINT32*)ptr;
				UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
				UINT8 masked = count & 31;

				if (masked)
				{
					auto result = (UINT32)((INT32)dest >> masked);
					*(UINT32*)ptr = result;
					FLAGS.CF = (dest >> (masked - 1)) & 1;
					if (masked == 1)
						FLAGS.OF = (dest >> 31) & 1;
				}
			}
			else
			{
				auto dest = *(UINT16*)ptr;
				UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
				UINT8 masked = count & 15;

				if (masked)
				{
					auto result = (UINT16)((INT16)dest >> masked);
					*(UINT16*)ptr = result;
					FLAGS.CF = (dest >> (masked - 1)) & 1;
					if (masked == 1)
						FLAGS.OF = (dest >> 15) & 1;
				}
			}
			status = true;
		}
	}break;
	}

	return status;
}

