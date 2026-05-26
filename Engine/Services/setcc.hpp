#pragma once

bool AssemblyState::service_setcc()
{
	bool status = false;

	switch (*RIP)
	{
	case 0x0F:
	{
		RIP++;
		switch (*RIP)
		{
		case 0x90:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = FLAGS.OF ? 1 : 0;
				status = true;
			}
		}break;
		case 0x91:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = !FLAGS.OF ? 1 : 0;
				status = true;
			}
		}break;
		case 0x92:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = FLAGS.CF ? 1 : 0;
				status = true;
			}
		}break;
		case 0x93:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = !FLAGS.CF ? 1 : 0;
				status = true;
			}
		}break;
		case 0x94:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = FLAGS.ZF ? 1 : 0;
				status = true;
			}
		}break;
		case 0x95:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = !FLAGS.ZF ? 1 : 0;
				status = true;
			}
		}break;
		case 0x96:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = (FLAGS.CF || FLAGS.ZF) ? 1 : 0;
				status = true;
			}
		}break;
		case 0x97:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = !(FLAGS.CF || FLAGS.ZF) ? 1 : 0;
				status = true;
			}
		}break;
		case 0x98:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = FLAGS.SF ? 1 : 0;
				status = true;
			}
		}break;
		case 0x99:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = !FLAGS.SF ? 1 : 0;
				status = true;
			}
		}break;
		case 0x9a:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = FLAGS.PF ? 1 : 0;
				status = true;
			}
		}break;
		case 0x9b:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = !FLAGS.PF ? 1 : 0;
				status = true;
			}

		}break;
		case 0x9c:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = (FLAGS.SF != FLAGS.OF) ? 1 : 0;
				status = true;
			}
		}break;
		case 0x9d:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = !(FLAGS.SF != FLAGS.OF) ? 1 : 0;
				status = true;
			}
		}break;
		case 0x9e:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = (FLAGS.ZF || FLAGS.SF != FLAGS.OF) ? 1 : 0;
				status = true;
			}
		}break;
		case 0x9f:
		{
			auto ptr = GetDisplacementPtr();
			if (ptr)
			{
				*(UINT8*)ptr = !(FLAGS.ZF || FLAGS.SF != FLAGS.OF) ? 1 : 0;
				status = true;
			}
		}break;
		};
	}break;
	};

	if(status)
		printf("setcc\n");

	return status;
}

