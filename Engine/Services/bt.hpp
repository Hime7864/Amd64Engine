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
		case 0xA3:// r/m16/32/64	r16/32/64
		{
			auto modrm = (MODRM*)(&RIP[1]);
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)
				{
					FLAGS.CF = read_bit(
						*(UINT64*)ptr, 
						*(UINT64*)&GPR[modrm_register], 
						64);
				}
				else if (!Prefix.OperandSize)
				{
					FLAGS.CF = read_bit(
						*(UINT32*)ptr, 
						*(UINT32*)&GPR[modrm_register], 
						32);
				}
				else
				{
					FLAGS.CF = read_bit(
						*(UINT16*)ptr, 
						*(UINT16*)&GPR[modrm_register],
						16);
				}
				status = true;
			}
		}break;
		case 0xAB:// r/m16/32/64	r16/32/64
		{
			auto modrm = (MODRM*)(&RIP[1]);
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)
				{
					FLAGS.CF = read_bit(
						*(UINT64*)ptr,
						*(UINT64*)&GPR[modrm_register],
						64);
					write_bit(
						*(UINT64*)ptr,
						*(UINT64*)&GPR[modrm_register],
						64,
						true);
				}
				else if (!Prefix.OperandSize)
				{
					FLAGS.CF = read_bit(
						*(UINT32*)ptr,
						*(UINT32*)&GPR[modrm_register],
						32);
					write_bit(
						*(UINT32*)ptr,
						*(UINT32*)&GPR[modrm_register],
						32,
						true);
				}
				else
				{
					FLAGS.CF = read_bit(
						*(UINT16*)ptr,
						*(UINT16*)&GPR[modrm_register],
						16);
					write_bit(
						*(UINT16*)ptr,
						*(UINT16*)&GPR[modrm_register],
						16,
						true);
				}
				status = true;
			}
		}break;
		case 0xB3:// r/m16/32/64	r16/32/64
		{
			auto modrm = (MODRM*)(&RIP[1]);
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)
				{
					FLAGS.CF = read_bit(
						*(UINT64*)ptr,
						*(UINT64*)&GPR[modrm_register],
						64);
					write_bit(
						*(UINT64*)ptr,
						*(UINT64*)&GPR[modrm_register],
						64,
						false);
				}
				else if (!Prefix.OperandSize)
				{
					FLAGS.CF = read_bit(
						*(UINT32*)ptr,
						*(UINT32*)&GPR[modrm_register],
						32);
					write_bit(
						*(UINT32*)ptr,
						*(UINT32*)&GPR[modrm_register],
						32,
						false);
				}
				else
				{
					FLAGS.CF = read_bit(
						*(UINT16*)ptr,
						*(UINT16*)&GPR[modrm_register],
						16);
					write_bit(
						*(UINT16*)ptr,
						*(UINT16*)&GPR[modrm_register],
						16,
						false);
				}
				status = true;
			}
		}break;
		case 0xBA:// r/m16/32/64	imm8
		{
			auto modrm = (MODRM*)(&RIP[1]);
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto imm = *(UINT8*)(&RIP[1]);
				if (Prefix.W)
				{
					FLAGS.CF = read_bit(
						*(UINT64*)ptr,
						imm,
						64);
					switch (modrm->Register)
					{
					case 5:
					{
						write_bit(
							*(UINT64*)ptr,
							imm,
							64,
							true);
					}break;
					case 6:
					{
						write_bit(
							*(UINT64*)ptr,
							imm,
							64,
							false);
					}break;
					case 7:
					{
						write_bit(
							*(UINT64*)ptr,
							imm,
							64,
							!FLAGS.CF);
					}break;
					};
				}
				else if (!Prefix.OperandSize)
				{
					FLAGS.CF = read_bit(
						*(UINT32*)ptr,
						imm,
						32);
					switch (modrm->Register)
					{
					case 5:
					{
						write_bit(
							*(UINT32*)ptr,
							imm,
							32,
							true);
					}break;
					case 6:
					{
						write_bit(
							*(UINT32*)ptr,
							imm,
							32,
							false);
					}break;
					case 7:
					{
						write_bit(
							*(UINT32*)ptr,
							imm,
							32,
							!FLAGS.CF);
					}break;
					};
				}
				else
				{
					FLAGS.CF = read_bit(
						*(UINT16*)ptr,
						imm,
						16);
					switch (modrm->Register)
					{
					case 5:
					{
						write_bit(
							*(UINT16*)ptr,
							imm,
							16,
							true);
					}break;
					case 6:
					{
						write_bit(
							*(UINT16*)ptr,
							imm,
							16,
							false);
					}break;
					case 7:
					{
						write_bit(
							*(UINT16*)ptr,
							imm,
							16,
							!FLAGS.CF);
					}break;
					};
				}
				RIP++;
				status = true;
			}
		}break;
		case 0xBB:// r/m16/32/64	r16/32/64
		{
			auto modrm = (MODRM*)(&RIP[1]);
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				auto modrm_register = Prefix.R ? modrm->Register + 8 : modrm->Register;
				if (Prefix.W)
				{
					FLAGS.CF = read_bit(
						*(UINT64*)ptr,
						*(UINT64*)&GPR[modrm_register],
						64);
					write_bit(
						*(UINT64*)ptr,
						*(UINT64*)&GPR[modrm_register],
						64,
						!FLAGS.CF);
				}
				else if (!Prefix.OperandSize)
				{
					FLAGS.CF = read_bit(
						*(UINT32*)ptr,
						*(UINT32*)&GPR[modrm_register],
						32);
					write_bit(
						*(UINT32*)ptr,
						*(UINT32*)&GPR[modrm_register],
						32,
						!FLAGS.CF);
				}
				else
				{
					FLAGS.CF = read_bit(
						*(UINT16*)ptr,
						*(UINT16*)&GPR[modrm_register],
						16);
					write_bit(
						*(UINT16*)ptr,
						*(UINT16*)&GPR[modrm_register],
						16,
						!FLAGS.CF);
				}
				status = true;
			}
		}break;
		};
	}break;
	};

	return status;
}

