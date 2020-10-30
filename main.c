
#include "stm32f4xx.h"
#include "test.h"


void led_init()
{ //PC13
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
}

int main(void)
{

	uint8_t result = add1(1,2);

//	led_init();
//	volatile uint32_t delay;
//	while(1){
//	GPIOC->ODR |= GPIO_ODR_ODR_13;
//	for(delay = 1000000; delay; delay--){};
//	GPIOC->ODR &= ~GPIO_ODR_ODR_13;
//	for(delay = 1000000; delay; delay--){};
//	}
}
