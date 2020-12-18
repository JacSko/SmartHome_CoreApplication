#include "stm32f4xx.h"

#define irq_arr_size 5

uint8_t active_irq[irq_arr_size];
uint8_t active_irq_size = 0;

void stm_stub_init()
{
	GPIOA = (GPIO_TypeDef*) calloc(1, sizeof(GPIO_TypeDef));
	GPIOB = (GPIO_TypeDef*) calloc(1, sizeof(GPIO_TypeDef));
	GPIOC = (GPIO_TypeDef*) calloc(1, sizeof(GPIO_TypeDef));
	RCC = (RCC_TypeDef*) calloc(1, sizeof(RCC_TypeDef));
	USART1 = (USART_TypeDef*) calloc(1, sizeof(USART_TypeDef));
	USART2 = (USART_TypeDef*) calloc(1, sizeof(USART_TypeDef));
	NVIC = (NVIC_Type*) calloc(1, sizeof(NVIC_Type));
	SYSCFG = (SYSCFG_TypeDef*) calloc(1, sizeof(SYSCFG_TypeDef));
	EXTI = (EXTI_TypeDef*) calloc(1, sizeof(EXTI_TypeDef));
	TIM2 = (TIM_TypeDef*) calloc(1, sizeof(TIM_TypeDef));
	I2C1 = (I2C_TypeDef*) calloc(1, sizeof(I2C_TypeDef));

	for (uint8_t i = 0; i < irq_arr_size; i++)
	{
		active_irq[i] = 0x00;
	}
	active_irq_size = 0;
}
void stm_stub_deinit()
{
	free(GPIOA);
	free(GPIOB);
	free(GPIOC);
	free(RCC);
	free(USART1);
	free(USART2);
	free(NVIC);
	free(SYSCFG);
	free(EXTI);
	free(TIM2);
	free(I2C1);
}
void NVIC_EnableIRQ(IRQn_Type IRQn)
{
	active_irq[active_irq_size] = IRQn;
	active_irq_size++;
	if (active_irq_size > irq_arr_size)
	{
		active_irq_size = 0;
	}
}

void NVIC_DisableIRQ(IRQn_Type IRQn)
{
	for (uint8_t i = 0; i < irq_arr_size; i++)
	{
		if (active_irq[i] == IRQn)
		{
			active_irq[i] = 0;
		}
	}
}

uint8_t stm_stub_check_irq(IRQn_Type IRQn, uint8_t active)
{
	uint8_t result = 0x01;

	for (uint8_t i = 0; i < irq_arr_size; i++)
	{
		if (active_irq[i] == IRQn)
		{
			if (active)
			{
				result = 0x01;
			}
			else
			{
				result = 0x00;
			}
			break;
		}
	}

	return result;
}

void SysTick_Config(uint32_t ticks)
{

}

void __DSB(void)
{

}
