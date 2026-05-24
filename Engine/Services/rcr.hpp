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
			auto dest = *(UINT8*)ptr;
			UINT8 count = RIP[2];
			UINT8 masked = count & 7;

			if (masked)
			{
				UINT8 old_cf = FLAGS.CF;
				UINT8 result = dest;
				UINT8 cf = old_cf;
				for (UINT8 i = 0; i < masked; i++)
				{
					UINT8 new_cf = result & 1;
					result = (result >> 1) | (cf << 7);
					cf = new_cf;
				}
				FLAGS.CF = cf;
				*(UINT8*)ptr = result;
				if (masked == 1)
					FLAGS.OF = ((dest >> 7) & 1) ^ old_cf;
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
				UINT8 count = RIP[2];
				UINT8 masked = count & 63;

				if (masked)
				{
					UINT64 old_cf = FLAGS.CF;
					UINT64 result = dest;
					UINT64 cf = old_cf;
					for (UINT8 i = 0; i < masked; i++)
					{
						UINT64 new_cf = result & 1;
						result = (result >> 1) | (cf << 63);
						cf = new_cf;
					}
					FLAGS.CF = cf;
					*(UINT64*)ptr = result;
					if (masked == 1)
						FLAGS.OF = ((dest >> 63) & 1) ^ old_cf;
				}
			}
			else if (!Prefix.OperandSize)
			{
				auto dest = *(UINT32*)ptr;
				UINT8 count = RIP[2];
				UINT8 masked = count & 31;

				if (masked)
				{
					UINT32 old_cf = FLAGS.CF;
					UINT32 result = dest;
					UINT32 cf = old_cf;
					for (UINT8 i = 0; i < masked; i++)
					{
						UINT32 new_cf = result & 1;
						result = (result >> 1) | (cf << 31);
						cf = new_cf;
					}
					FLAGS.CF = cf;
					*(UINT32*)ptr = result;
					if (masked == 1)
						FLAGS.OF = ((dest >> 31) & 1) ^ old_cf;
				}
			}
			else
			{
				auto dest = *(UINT16*)ptr;
				UINT8 count = RIP[2];
				UINT8 masked = count & 15;

				if (masked)
				{
					UINT16 old_cf = FLAGS.CF;
					UINT16 result = dest;
					UINT16 cf = old_cf;
					for (UINT8 i = 0; i < masked; i++)
					{
						UINT16 new_cf = result & 1;
						result = (result >> 1) | (cf << 15);
						cf = new_cf;
					}
					FLAGS.CF = cf;
					*(UINT16*)ptr = result;
					if (masked == 1)
						FLAGS.OF = ((dest >> 15) & 1) ^ old_cf;
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
				UINT8 old_cf = FLAGS.CF;
				UINT8 result = dest;
				UINT8 cf = old_cf;
				for (UINT8 i = 0; i < masked; i++)
				{
					UINT8 new_cf = result & 1;
					result = (result >> 1) | (cf << 7);
					cf = new_cf;
				}
				FLAGS.CF = cf;
				*(UINT8*)ptr = result;
				if (masked == 1)
					FLAGS.OF = ((dest >> 7) & 1) ^ old_cf;
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
					UINT64 old_cf = FLAGS.CF;
					UINT64 result = dest;
					UINT64 cf = old_cf;
					for (UINT8 i = 0; i < masked; i++)
					{
						UINT64 new_cf = result & 1;
						result = (result >> 1) | (cf << 63);
						cf = new_cf;
					}
					FLAGS.CF = cf;
					*(UINT64*)ptr = result;
					if (masked == 1)
						FLAGS.OF = ((dest >> 63) & 1) ^ old_cf;
				}
			}
			else if (!Prefix.OperandSize)
			{
				auto dest = *(UINT32*)ptr;
				UINT8 count = 1;
				UINT8 masked = count & 31;

				if (masked)
				{
					UINT32 old_cf = FLAGS.CF;
					UINT32 result = dest;
					UINT32 cf = old_cf;
					for (UINT8 i = 0; i < masked; i++)
					{
						UINT32 new_cf = result & 1;
						result = (result >> 1) | (cf << 31);
						cf = new_cf;
					}
					FLAGS.CF = cf;
					*(UINT32*)ptr = result;
					if (masked == 1)
						FLAGS.OF = ((dest >> 31) & 1) ^ old_cf;
				}
			}
			else
			{
				auto dest = *(UINT16*)ptr;
				UINT8 count = 1;
				UINT8 masked = count & 15;

				if (masked)
				{
					UINT16 old_cf = FLAGS.CF;
					UINT16 result = dest;
					UINT16 cf = old_cf;
					for (UINT8 i = 0; i < masked; i++)
					{
						UINT16 new_cf = result & 1;
						result = (result >> 1) | (cf << 15);
						cf = new_cf;
					}
					FLAGS.CF = cf;
					*(UINT16*)ptr = result;
					if (masked == 1)
						FLAGS.OF = ((dest >> 15) & 1) ^ old_cf;
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
				UINT8 old_cf = FLAGS.CF;
				UINT8 result = dest;
				UINT8 cf = old_cf;
				for (UINT8 i = 0; i < masked; i++)
				{
					UINT8 new_cf = result & 1;
					result = (result >> 1) | (cf << 7);
					cf = new_cf;
				}
				FLAGS.CF = cf;
				*(UINT8*)ptr = result;
				if (masked == 1)
					FLAGS.OF = ((dest >> 7) & 1) ^ old_cf;
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
					UINT64 old_cf = FLAGS.CF;
					UINT64 result = dest;
					UINT64 cf = old_cf;
					for (UINT8 i = 0; i < masked; i++)
					{
						UINT64 new_cf = result & 1;
						result = (result >> 1) | (cf << 63);
						cf = new_cf;
					}
					FLAGS.CF = cf;
					*(UINT64*)ptr = result;
					if (masked == 1)
						FLAGS.OF = ((dest >> 63) & 1) ^ old_cf;
				}
			}
			else if (!Prefix.OperandSize)
			{
				auto dest = *(UINT32*)ptr;
				UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
				UINT8 masked = count & 31;

				if (masked)
				{
					UINT32 old_cf = FLAGS.CF;
					UINT32 result = dest;
					UINT32 cf = old_cf;
					for (UINT8 i = 0; i < masked; i++)
					{
						UINT32 new_cf = result & 1;
						result = (result >> 1) | (cf << 31);
						cf = new_cf;
					}
					FLAGS.CF = cf;
					*(UINT32*)ptr = result;
					if (masked == 1)
						FLAGS.OF = ((dest >> 31) & 1) ^ old_cf;
				}
			}
			else
			{
				auto dest = *(UINT16*)ptr;
				UINT8 count = *(UINT8*)&GPR[(int)EGPR::RCX];
				UINT8 masked = count & 15;

				if (masked)
				{
					UINT16 old_cf = FLAGS.CF;
					UINT16 result = dest;
					UINT16 cf = old_cf;
					for (UINT8 i = 0; i < masked; i++)
					{
						UINT16 new_cf = result & 1;
						result = (result >> 1) | (cf << 15);
						cf = new_cf;
					}
					FLAGS.CF = cf;
					*(UINT16*)ptr = result;
					if (masked == 1)
						FLAGS.OF = ((dest >> 15) & 1) ^ old_cf;
				}
			}
			status = true;
		}
	}break;
	}

	return status;
}

