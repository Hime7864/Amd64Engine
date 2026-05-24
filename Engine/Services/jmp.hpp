#pragma once

bool AssemblyState::service_jmp()
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
		case 0x80:
			state = FLAGS.OF;
			break;
		case 0x81:
			state = !FLAGS.OF;
			break;
		case 0x82:
			state = FLAGS.CF;
			break;
		case 0x83:
			state = !FLAGS.CF;
			break;
		case 0x84:
			state = FLAGS.ZF;
			break;
		case 0x85:
			state = !FLAGS.ZF;
			break;
		case 0x86:
			state = FLAGS.CF || FLAGS.ZF;
			break;
		case 0x87:
			state = (!FLAGS.CF && !FLAGS.ZF);
			break;
		case 0x88:
			state = FLAGS.SF;
			break;
		case 0x89:
			state = !FLAGS.SF;
			break;
		case 0x8A:
			state = FLAGS.PF;
			break;
		case 0x8B:
			state = !FLAGS.PF;
			break;
		case 0x8C:
			state = FLAGS.SF != FLAGS.OF;
			break;
		case 0x8D:
			state = FLAGS.SF == FLAGS.OF;
			break;
		case 0x8E:
			state = FLAGS.ZF && (FLAGS.SF != FLAGS.OF);
			break;
		case 0x8F:
			state = !FLAGS.ZF && (FLAGS.SF == FLAGS.OF);
			break;
		};
		
		if (*RIP >= 0x80 && *RIP <= 0x8F)
		{
			if (Prefix.OperandSize)
			{
				auto imm = *(INT16*)(&RIP[1]);
				if (state)
				{
					RIP += imm + 3;
				}
				else
				{
					RIP += 3;
				}
				status = true;
			}
			else
			{
				auto imm = *(INT32*)(&RIP[1]);
				if (state)
				{
					RIP += imm + 5;
				}
				else
				{
					RIP += 5;
				}
				status = true;
			}
		}
	}break;
	case 0x70:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.OF)
		{
			RIP += imm + 2; 
		}
		else
		{
			RIP += 2;
		}
		status = true;
	}break;
	case 0x71:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.OF)
		{
			RIP += 2;
		}
		else
		{
			RIP += imm + 2;
		}
		status = true;
	}break;
	case 0x72:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.CF)
		{
			RIP += imm + 2;
		}
		else
		{
			RIP += 2;
		}
		status = true;
	}break;
	case 0x73:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.CF)
		{
			RIP += 2;
		}
		else
		{
			RIP += imm + 2;
		}
		status = true;
	}break;
	case 0x74:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.ZF)
		{
			RIP += imm + 2;
			
		}
		else
		{
			RIP += 2;
		}
		status = true;
	}break;
	case 0x75:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.ZF)
		{
			RIP += 2;
		}
		else
		{
			RIP += imm + 2;
		}
		status = true;
	}break;
	case 0x76:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.CF || FLAGS.ZF)
		{
			RIP += imm + 2;

		}
		else
		{
			RIP += 2;
		}
		status = true;
	}break;
	case 0x77:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.CF == 0 && FLAGS.ZF == 0)
		{
			RIP += imm + 2;

		}
		else
		{
			RIP += 2;
		}
		status = true;
	}break;
	case 0x78:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.SF)
		{
			RIP += imm + 2;

		}
		else
		{
			RIP += 2;
		}
		status = true;
	}break;
	case 0x79:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.SF)
		{
			RIP += 2;
		}
		else
		{
			RIP += imm + 2;
		}
		status = true;
	}break;
	case 0x7A:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.PF)
		{
			RIP += imm + 2;

		}
		else
		{
			RIP += 2;
		}
		status = true;
	}break;
	case 0x7B:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.PF)
		{
			RIP += 2;
		}
		else
		{
			RIP += imm + 2;
		}
		status = true;
	}break;
	case 0x7C:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.SF != FLAGS.OF)
		{
			RIP += imm + 2;

		}
		else
		{
			RIP += 2;
		}
		status = true;
	}break;
	case 0x7D:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.SF == FLAGS.OF)
		{
			RIP += 2;
		}
		else
		{
			RIP += imm + 2;
		}
		status = true;
	}break;
	case 0x7E:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.ZF || FLAGS.SF != FLAGS.OF)
		{
			RIP += imm + 2;

		}
		else
		{
			RIP += 2;
		}
		status = true;
	}break;
	case 0x7F:
	{
		auto imm = *(INT8*)(&RIP[1]);
		if (FLAGS.ZF && FLAGS.SF == FLAGS.OF)
		{
			RIP += 2;
		}
		else
		{
			RIP += imm + 2;
		}
		status = true;
	}break;
	case 0xE9:
	{
		auto imm = *(INT32*)(&RIP[1]);
		RIP += imm + 5;
		status = true;
	}break;
	case 0xEB:
	{
		auto imm = *(INT8*)(&RIP[1]);
		RIP += imm + 2;
		status = true;
	}break;
	case 0xFF:
	{
		auto modrm = (MODRM*)(&RIP[1]);
		switch (modrm->Register)
		{
		case 4:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{

				if (Prefix.OperandSize)
				{
					RIP = (BYTE*)*(UINT16*)ptr;
					status = true;
				}
				else
				{
					RIP = (BYTE*)*(UINT64*)ptr;
					status = true;
				}
			}
		}break;
		case 5:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{

				RIP = (BYTE*)*(UINT64*)ptr;
				status = true;
			}
		}break;
		}
	}break;
	};

	return status;
}

