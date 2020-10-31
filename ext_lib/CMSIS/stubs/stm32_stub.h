

#ifndef EXT_LIB_CMSIS_STUBS_STM32_STUB_H_
#define EXT_LIB_CMSIS_STUBS_STM32_STUB_H_

#include "stm32f4xx.h"

void stm_stub_init()
{
	GPIOC = new GPIO_TypeDef();
}

void stm_stub_deinit()
{
	delete GPIOC;

}




#endif /* EXT_LIB_CMSIS_STUBS_STM32_STUB_H_ */
