#include "stm32f4xx.h"
#include "inputs_board.h"
#include "gpio_lib.h"

void int_handler_init()
{
   RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
   RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
   __DSB();
   gpio_pin_cfg(GPIOB, PB4, gpio_mode_in_floating);
   GPIOB->ODR |= GPIO_ODR_ODR_4;
   SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PB;
   EXTI->IMR |= EXTI_IMR_MR4;
   EXTI->RTSR &= ~EXTI_RTSR_TR4;
   EXTI->FTSR |= EXTI_FTSR_TR4; /* interrupt active only for falling edge */
   NVIC_EnableIRQ(EXTI4_IRQn);
}

void EXTI4_IRQHandler()
{
   if ( EXTI->PR & EXTI_PR_PR4) {
      EXTI->PR = EXTI_PR_PR4;
      inp_on_interrupt_recevied();
      return;
   }
}
