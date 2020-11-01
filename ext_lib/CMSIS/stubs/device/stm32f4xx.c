#include "stm32f4xx.h"

void stm_stub_init()
{
	GPIOA = (GPIO_TypeDef*) malloc(sizeof(GPIO_TypeDef));
	GPIOB = (GPIO_TypeDef*) malloc(sizeof(GPIO_TypeDef));
	GPIOC = (GPIO_TypeDef*) malloc(sizeof(GPIO_TypeDef));
	RCC = (RCC_TypeDef*) malloc(sizeof(RCC_TypeDef));
	USART1 = (USART_TypeDef*) malloc(sizeof(USART_TypeDef));
	NVIC = (NVIC_Type*) malloc(sizeof(NVIC_Type));
}
void stm_stub_deinit()
{
	free(GPIOA);
	free(GPIOB);
	free(GPIOC);
	free(RCC);
	free(USART1);
	free(NVIC);
}
void NVIC_EnableIRQ(IRQn_Type IRQn)
{

}
void __DSB(void)
{

}
