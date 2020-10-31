#include "test.h"

uint8_t add1 (uint8_t arg1, uint8_t arg2)
{
	GPIOC->IDR = 0x00;
}

uint8_t add2 (uint8_t arg1, uint8_t arg2)
{
	if (arg1>arg2)
	{
		GPIOC->IDR = 0xFF;
	}

}
