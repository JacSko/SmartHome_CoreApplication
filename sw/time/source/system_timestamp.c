/* =============================
 *   Includes of common headers
 * =============================*/
/* =============================
 *  Includes of project headers
 * =============================*/
#include "system_timestamp.h"
#include "stm32f4xx.h"
/* =============================
 *      Module variables
 * =============================*/
volatile uint16_t system_timestamp;




void ts_init()
{
   RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
   __DSB();
   TIM4->PSC = 1000;
   TIM4->ARR = 10000;
   TIM4->CR1 |= TIM_CR1_CEN;
   TIM4->CNT = 0;
   TIM4->DIER |= TIM_DIER_UIE;
   system_timestamp = 0;
   NVIC_EnableIRQ(TIM4_IRQn);
}
void ts_deinit()
{
   NVIC_DisableIRQ(TIM4_IRQn);
   TIM4->CR1 &= ~TIM_CR1_CEN;
   TIM4->CNT = 0;
   system_timestamp = 0;
}
uint16_t ts_get()
{
   return system_timestamp;
}

void ts_wait(uint16_t ms)
{
   uint16_t ts = ts_get();
   while(ts_get_diff(ts) < ms);
}
uint16_t ts_get_diff(uint16_t timestamp)
{
   uint16_t result = 0;
   if (timestamp <= system_timestamp)
   {
      result = system_timestamp - (timestamp);
   }
   else
   {
      result = system_timestamp + (UINT16_MAX - (timestamp));
   }
   return result * 100;
}
void TIM4_IRQHandler()
{
   if (TIM4->SR & TIM_SR_UIF){
      TIM4->SR &= ~TIM_SR_UIF;
      system_timestamp++;
   }
}
