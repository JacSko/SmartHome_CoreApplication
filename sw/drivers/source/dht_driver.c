/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdlib.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "stm32f4xx.h"
#include "dht_driver.h"
#include "gpio_lib.h"
#include "task_scheduler.h"
/* =============================
 *          Defines
 * =============================*/
#define DHT_MINIMUM_TIMEOUT_MS 30
#define DHT_MAXIMUM_TIMEOUT_MS 300
/* =============================
 *   Internal module functions
 * =============================*/
void dht_on_timeout();
RET_CODE dht_verify_timeout(uint16_t);
DHT_STATUS dht_decode_data(DHT_SENSOR* sensor);
RET_CODE dht_send_start();
void dht_set_gpio_state(DHT_SENSOR_ID id, uint8_t state);
/* =============================
 *       Internal types
 * =============================*/
typedef enum DHT_DRIVER_STATE
{
	DHT_STATE_IDLE,
	DHT_STATE_START,
	DHT_STATE_READING,
	DHT_STATE_TIMEOUT,
	DHT_STATE_CHECKSUM_ERROR,
} DHT_DRIVER_STATE;
typedef struct DHT_DRIVER
{
	uint16_t timeout;
	uint8_t measurement_ongoing;
	uint8_t raw_data[5];
	DHT_DRIVER_STATE state;
	DHT_SENSOR_ID sensor;
} DHT_DRIVER;
/* =============================
 *      Module variables
 * =============================*/
uint8_t DHT_MEASURE_TIMESTAMPS[2 + 40]; /*2x TS (DHT presence response + sending start) + 40 bits of data */
volatile uint8_t dht_timestamp_idx;
DHT_DRIVER driver;
DHT_CALLBACK callback;


RET_CODE dht_initialize()
{
   RET_CODE result = RETURN_NOK;

	/*
	 * 	The STM peripherial initialization.
	 * 	All GPIOs used for sensors are confiured in default:
	 * 	- as output (switched to input after start sequence),
	 * 	- interrupts only on rising edge (falling edge disabled)
	 * 	TIM2 configured to ensure 1us period beteen interrupts fires
	 */

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	__DSB();
	gpio_pin_cfg(GPIOB, PB9, gpio_mode_output_OD_HS);
	gpio_pin_cfg(GPIOB, PB10, gpio_mode_output_OD_HS);
	gpio_pin_cfg(GPIOB, PB12, gpio_mode_output_OD_HS);
	gpio_pin_cfg(GPIOB, PB13, gpio_mode_output_OD_HS);
	gpio_pin_cfg(GPIOB, PB14, gpio_mode_output_OD_HS);
	gpio_pin_cfg(GPIOB, PB15, gpio_mode_output_OD_HS);
	GPIOB->ODR |= GPIO_ODR_ODR_9;
	GPIOB->ODR |= GPIO_ODR_ODR_10;
	GPIOB->ODR |= GPIO_ODR_ODR_12;
	GPIOB->ODR |= GPIO_ODR_ODR_13;
	GPIOB->ODR |= GPIO_ODR_ODR_14;
	GPIOB->ODR |= GPIO_ODR_ODR_15;

	SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI9_PB;
	SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI10_PB;
	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI12_PB;
	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PB;
	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI14_PB;
	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI15_PB;
	EXTI->IMR |= EXTI_IMR_MR9;
	EXTI->IMR |= EXTI_IMR_MR10;
	EXTI->IMR |= EXTI_IMR_MR12;
	EXTI->IMR |= EXTI_IMR_MR13;
	EXTI->IMR |= EXTI_IMR_MR14;
	EXTI->IMR |= EXTI_IMR_MR15;
	EXTI->RTSR |= EXTI_RTSR_TR9;
	EXTI->RTSR |= EXTI_RTSR_TR10;
	EXTI->RTSR |= EXTI_RTSR_TR12;
	EXTI->RTSR |= EXTI_RTSR_TR13;
	EXTI->RTSR |= EXTI_RTSR_TR14;
	EXTI->RTSR |= EXTI_RTSR_TR15;
	EXTI->FTSR &= ~EXTI_FTSR_TR9;
	EXTI->FTSR &= ~EXTI_FTSR_TR10;
	EXTI->FTSR &= ~EXTI_FTSR_TR12;
	EXTI->FTSR &= ~EXTI_FTSR_TR13;
	EXTI->FTSR &= ~EXTI_FTSR_TR14;
	EXTI->FTSR &= ~EXTI_FTSR_TR15;

	/*TIM2 is clocked by APB1 - this is 50MHz, increment time - 1us */
	/* 0 bit - 50us + 28us = 78us */
	/* 1 bit - 50us + 70us = 120us */
	/* 2 + 40 timestamps have to be collected */
	//TIM2->CR1 |= TIM_CR1_OPM;
	TIM2->DIER |= TIM_DIER_UIE;
	TIM2->ARR = 10;
	TIM2->PSC = 100;
	NVIC_EnableIRQ(TIM2_IRQn);
	TIM2->CR1 |= TIM_CR1_CEN;

	if (sch_subscribe_and_set(&dht_on_timeout, TASKPRIO_HIGH, 20, TASKSTATE_STOPPED, TASKTYPE_TRIGGER) == RETURN_OK)
	{
	   driver.state = DHT_STATE_IDLE;
	   result = RETURN_OK;
	}
	return result;
}

RET_CODE dht_read_async(DHT_SENSOR_ID id, DHT_CALLBACK clb)
{
   RET_CODE result = RETURN_NOK;
	dht_timestamp_idx = 0;
	callback = clb;
	if (driver.state == DHT_STATE_IDLE && id < DHT_ENUM_MAX)
	{
	   driver.sensor = id;
	   result = dht_send_start();
	}
	return result;
}

RET_CODE dht_send_start()
{
   RET_CODE result = RETURN_NOK;

   do
   {
      if (sch_set_task_period(&dht_on_timeout, 20) != RETURN_OK)
      {
         break;
      }
      if (sch_trigger_task(&dht_on_timeout) != RETURN_OK)
      {
         break;
      }
      dht_set_gpio_state(driver.sensor, 0);
      result = RETURN_OK;
      driver.state = DHT_STATE_START;
   }while(0);

}

void dht_set_gpio_state(DHT_SENSOR_ID id, uint8_t state)
{
   switch (id)
   {/* TODO set correct gpio here */
   case DHT_SENSOR1:
      break;
   case DHT_SENSOR2:
      break;
   case DHT_SENSOR3:
      break;
   case DHT_SENSOR4:
      break;
   case DHT_SENSOR5:
      break;
   case DHT_SENSOR6:
      break;
   }
}

DHT_STATUS dht_read(DHT_SENSOR_ID id, DHT_SENSOR* sensor)
{
	DHT_STATUS result = DHT_STATUS_UNKNOWN;

	if (dht_read_async(id, NULL) == RETURN_OK)
	{
	   while(driver.state <= DHT_STATE_READING);
	   if (driver.state == DHT_STATE_TIMEOUT)
	   {
	      result = DHT_STATUS_NO_RESPONSE;
	   }
	   else
	   {
	      result = dht_decode_data(sensor);
	   }
	   return result;
	}
	return result;
}

RET_CODE dht_set_timeout(uint16_t timeout)
{
	RET_CODE result = RETURN_NOK;
	if (dht_verify_timeout(timeout) == RETURN_OK)
	{
		result = RETURN_OK;
	}
	return result;
}

RET_CODE dht_verify_timeout(uint16_t period)
{
	RET_CODE result = RETURN_NOK;
	if (period >= DHT_MINIMUM_TIMEOUT_MS && period <= DHT_MAXIMUM_TIMEOUT_MS)
	{
		result = RETURN_OK;
	}
	return result;
}

uint16_t dht_get_timeout()
{
	return driver.timeout;
}

void dht_on_timeout()
{
   switch (driver.state)
   {
   case DHT_STATE_START:
      dht_set_gpio_state(driver.sensor, 1);
      //TODO enable interrupts here;
      sch_set_task_period(&dht_on_timeout, driver.timeout);
      sch_trigger_task(&dht_on_timeout);
      driver.state = DHT_STATE_READING;
      break;
   case DHT_STATE_READING:
      // TODO handle timeout
      driver.state = DHT_STATE_TIMEOUT;
      //disable interrupts
      break;
   default:
      break;
   }

}

DHT_STATUS dht_decode_data(DHT_SENSOR* sensor)
{

}

void EXTI15_10_IRQHandler()
{
	if ( EXTI->PR & EXTI_PR_PR13) {
		EXTI->PR = EXTI_PR_PR13;
		return;
	}
}

void EXTI9_5_IRQHandler()
{
	if ( EXTI->PR & EXTI_PR_PR13) {
		EXTI->PR = EXTI_PR_PR13;
		return;
	}
}
void TIM2_IRQHandler()
{
	if (TIM2->SR & TIM_SR_UIF)
	{
		TIM2->SR &= ~TIM_SR_UIF;
		GPIOB->ODR ^= GPIO_ODR_ODR_9;
		//TODO to be removed
		//PB9 should be switched after each 1us.
	}
}
