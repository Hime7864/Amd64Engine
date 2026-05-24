#pragma once

bool AssemblyState::service_dec()
{
	bool status = false;

	switch (*RIP)
	{
	case 0xFE:// r/m8
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			auto value = *(UINT8*)ptr;
			value--;
			*(UINT8*)ptr = value;

			FLAGS.SF = (value & 0x80) != 0;
			FLAGS.ZF = (value == 0);

			auto p = (UINT8)value;
			p ^= p >> 4;
			p ^= p >> 2;
			p ^= p >> 1;

			FLAGS.PF = (p & 1) == 0;
			FLAGS.OF = (value == 0x7F);
			status = true;
		}
	}break;
	case 0xFF:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		auto ptr = GetDisplacementPtr();
		if (ptr)
		{
			if (Prefix.W)
			{
				auto value = *(UINT64*)ptr;
				value--;
				*(UINT64*)ptr = value;

				FLAGS.SF = (value & 0x8000000000000000) != 0;
				FLAGS.ZF = (value == 0);

				auto p = (UINT8)value;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.OF = (value == 0x7FFFFFFFFFFFFFFF);
			}
			else if (!Prefix.OperandSize)
			{
				auto value = *(UINT32*)ptr;
				value--;
				*(UINT32*)ptr = value;

				FLAGS.SF = (value & 0x80000000) != 0;
				FLAGS.ZF = (value == 0);

				auto p = (UINT8)value;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0; 
			}
			else
			{
				auto value = *(UINT16*)ptr;
				value--;
				*(UINT16*)ptr = value;

				FLAGS.SF = (value & 0x8000) != 0;
				FLAGS.ZF = (value == 0);

				auto p = (UINT8)value;
				p ^= p >> 4;
				p ^= p >> 2;
				p ^= p >> 1;

				FLAGS.PF = (p & 1) == 0;
				FLAGS.OF = (value == 0x7FFF);
			}
			status = true;
		}
	}break;
	};

	return status;
}

