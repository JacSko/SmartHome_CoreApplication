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
#include "Logger.h"
/* =============================
 *          Defines
 * =============================*/
#define DHT_MINIMUM_TIMEOUT_MS 30
#define DHT_DEFAULT_TIMEOUT_MS 100
#define DHT_MAXIMUM_TIMEOUT_MS 300
#define DHT_START_TIME_MS 20
#define DHT_TIM_VALUE_LEVEL 10000
#define DHT_TIMESTAMPS_BUFFER_SIZE 42
#define DHT_TEMP_NEGATIVE_MASK 0x8000
/* =============================
 *   Internal module functions
 * =============================*/
void dht_on_timeout();
RET_CODE dht_verify_timeout(uint16_t);
DHT_STATUS dht_get_result();
RET_CODE dht_send_start();
void dht_set_gpio_state(DHT_SENSOR_ID id, uint8_t state);
RET_CODE dht_parse_data();
void dht_handle_exti_interrupt(uint32_t pr_mask);
void dht_handle_timestamp();
DHT_SENSOR_TYPE dht_get_sensor_type(const uint8_t* data);
/* =============================
 *       Internal types
 * =============================*/
typedef struct DHT_SENSOR_MASKS
{
   uint32_t gpio_odr_mask;
   uint32_t exti_pr_mask;
} DHT_SENSOR_MASKS;
typedef enum DHT_DRIVER_STATE
{
   DHT_STATE_IDLE,
   DHT_STATE_START,
   DHT_STATE_READING,
   DHT_STATE_DATA_DECODING,
   DHT_STATE_TIMEOUT,
   DHT_STATE_CHECKSUM_ERROR,
   DHT_STATE_ERROR,
} DHT_DRIVER_STATE;
typedef struct DHT_DRIVER
{
   uint16_t timeout;
   uint8_t raw_data[5];
   DHT_DRIVER_STATE state;
   DHT_SENSOR sensor;
} DHT_DRIVER;
/* =============================
 *      Module variables
 * =============================*/
uint32_t DHT_MEASURE_TIMESTAMPS[DHT_TIMESTAMPS_BUFFER_SIZE];
DHT_SENSOR_MASKS DHT_REG_MAP[DHT_ENUM_MAX];
volatile uint8_t dht_timestamp_idx;
volatile uint8_t dht_measurement_ready;
DHT_DRIVER dht_driver;
DHT_CALLBACK dht_callback;

RET_CODE dht_initialize()
{
   RET_CODE result = RETURN_NOK;

   /*
    * 	The STM peripherial initialization.
    * 	All GPIOs used for sensors are confiured:
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

   DHT_REG_MAP[DHT_SENSOR1].gpio_odr_mask = GPIO_ODR_ODR_9;
   DHT_REG_MAP[DHT_SENSOR2].gpio_odr_mask = GPIO_ODR_ODR_10;
   DHT_REG_MAP[DHT_SENSOR3].gpio_odr_mask = GPIO_ODR_ODR_12;
   DHT_REG_MAP[DHT_SENSOR4].gpio_odr_mask = GPIO_ODR_ODR_13;
   DHT_REG_MAP[DHT_SENSOR5].gpio_odr_mask = GPIO_ODR_ODR_14;
   DHT_REG_MAP[DHT_SENSOR6].gpio_odr_mask = GPIO_ODR_ODR_15;
   DHT_REG_MAP[DHT_SENSOR1].exti_pr_mask = EXTI_PR_PR9;
   DHT_REG_MAP[DHT_SENSOR2].exti_pr_mask = EXTI_PR_PR10;
   DHT_REG_MAP[DHT_SENSOR3].exti_pr_mask = EXTI_PR_PR12;
   DHT_REG_MAP[DHT_SENSOR4].exti_pr_mask = EXTI_PR_PR13;
   DHT_REG_MAP[DHT_SENSOR5].exti_pr_mask = EXTI_PR_PR14;
   DHT_REG_MAP[DHT_SENSOR6].exti_pr_mask = EXTI_PR_PR15;

   dht_set_gpio_state(DHT_SENSOR1, 1);
   dht_set_gpio_state(DHT_SENSOR2, 1);
   dht_set_gpio_state(DHT_SENSOR3, 1);
   dht_set_gpio_state(DHT_SENSOR4, 1);
   dht_set_gpio_state(DHT_SENSOR5, 1);
   dht_set_gpio_state(DHT_SENSOR6, 1);

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
   TIM2->ARR = 0xFFFFFFFF;
   TIM2->PSC = 100;
   NVIC_EnableIRQ(TIM2_IRQn);
   NVIC_EnableIRQ(EXTI9_5_IRQn);
   NVIC_EnableIRQ(EXTI15_10_IRQn);

   if (sch_subscribe_and_set(&dht_on_timeout, TASKPRIO_HIGH, DHT_START_TIME_MS,
         TASKSTATE_STOPPED, TASKTYPE_TRIGGER) == RETURN_OK)
   {
      dht_driver.state = DHT_STATE_IDLE;
      dht_driver.timeout = DHT_DEFAULT_TIMEOUT_MS;
      dht_measurement_ready = 0;
      result = RETURN_OK;
   }
   return result;
}

RET_CODE dht_read_async(DHT_SENSOR_ID id, DHT_CALLBACK clb)
{
   RET_CODE result = RETURN_NOK;
   dht_timestamp_idx = 0;
   dht_measurement_ready = 0;
   dht_callback = clb;

   if (dht_driver.state == DHT_STATE_IDLE && id < DHT_ENUM_MAX)
   {
      logger_send(LOG_ERROR, __func__, "Read sensor %d", id);
      dht_driver.sensor.id = id;
      result = dht_send_start();
   }
   return result;
}

RET_CODE dht_send_start()
{
   RET_CODE result = RETURN_NOK;

   do
   {
      if (sch_set_task_period(&dht_on_timeout, DHT_START_TIME_MS) != RETURN_OK)
      {
         logger_send(LOG_ERROR, __func__, "Cannot set task period");
         break;
      }
      if (sch_trigger_task(&dht_on_timeout) != RETURN_OK)
      {
         logger_send(LOG_ERROR, __func__, "Cannot start task");
         break;
      }
      dht_set_gpio_state(dht_driver.sensor.id, 0);
      result = RETURN_OK;
      dht_driver.state = DHT_STATE_START;
   } while (0);
   return result;

}

void dht_set_gpio_state(DHT_SENSOR_ID id, uint8_t state)
{
   if (state)
   {
      GPIOB->ODR |= DHT_REG_MAP[id].gpio_odr_mask;
   }
   else
   {
      GPIOB->ODR &= ~DHT_REG_MAP[id].gpio_odr_mask;
   }
}

DHT_STATUS dht_read(DHT_SENSOR_ID id, DHT_SENSOR *sensor)
{
   DHT_STATUS result = DHT_STATUS_UNKNOWN;

   if (dht_read_async(id, NULL) == RETURN_OK)
   {
      while(dht_measurement_ready == 0);
      dht_measurement_ready = 0;
      result = dht_get_result();
      *sensor = dht_driver.sensor;
   }
   return result;
}

RET_CODE dht_set_timeout(uint16_t timeout)
{
   RET_CODE result = RETURN_NOK;
   if (dht_verify_timeout(timeout) == RETURN_OK)
   {
      dht_driver.timeout = timeout;
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
   return dht_driver.timeout;
}

void dht_on_timeout()
{
   switch (dht_driver.state)
   {
   case DHT_STATE_START:
      dht_set_gpio_state(dht_driver.sensor.id, 1);
      if (sch_set_task_period(&dht_on_timeout, dht_driver.timeout) == RETURN_OK &&
          sch_trigger_task(&dht_on_timeout) == RETURN_OK)
      {
         TIM2->CR1 |= TIM_CR1_CEN;
         TIM2->CNT = 0;
         dht_driver.state = DHT_STATE_READING;
      }
      else
      {
         logger_send(LOG_ERROR, __func__, "Cannot start watchdog");
         dht_driver.state = DHT_STATE_ERROR;
         dht_measurement_ready = 1;
      }
      break;
   case DHT_STATE_READING:
      dht_driver.state = DHT_STATE_TIMEOUT;
      logger_send(LOG_ERROR, __func__, "Timeout");
      TIM2->CR1 &= ~TIM_CR1_CEN;
      TIM2->CNT = 0;
      dht_measurement_ready = 1;
      break;
   default:
      break;
   }

}

void dht_data_watcher()
{
   if (dht_measurement_ready == 1)
   {
      dht_measurement_ready = 0;
      DHT_STATUS result = dht_get_result();
      if (dht_callback)
      {
         dht_callback(result, &dht_driver.sensor);
      }
      dht_driver.state = DHT_STATE_IDLE;
   }
}

DHT_STATUS dht_get_result()
{
   DHT_STATUS result = DHT_STATUS_UNKNOWN;
   switch (dht_driver.state)
   {
   case DHT_STATE_DATA_DECODING:
      if (dht_parse_data() == RETURN_OK)
      {
         result = DHT_STATUS_OK;
      }
      else
      {
         result = DHT_STATUS_CHECKSUM_ERROR;
      }
      break;
   case DHT_STATE_TIMEOUT:
      result = DHT_STATUS_NO_RESPONSE;
      break;
   case DHT_STATE_ERROR:
      result = DHT_STATUS_ERROR;
      break;
   default:
      break;
   }
   return result;
}

RET_CODE dht_parse_data()
{
   RET_CODE result = RETURN_NOK;

   uint8_t byte_shift_idx = 7;
   uint8_t arr_idx = 0;
   uint8_t parsed_data [5] = {};
   for (uint8_t i = 2; i < DHT_TIMESTAMPS_BUFFER_SIZE; i++)
   {
      if (DHT_MEASURE_TIMESTAMPS[i] > DHT_TIM_VALUE_LEVEL)
      {
         parsed_data[arr_idx] |= (1 << byte_shift_idx);
      }
      if (byte_shift_idx == 0)
      {
         byte_shift_idx = 8;
         arr_idx++;
      }
      byte_shift_idx--;
   }

   uint8_t checksum = parsed_data[0] + parsed_data[1] + parsed_data[2] + parsed_data[3];

   if (checksum == parsed_data[4])
   {
      dht_driver.sensor.type = dht_get_sensor_type(parsed_data);
      switch(dht_driver.sensor.type)
      {
         case DHT_TYPE_DHT22:
         {
            int16_t hum = parsed_data[1];
            hum |= (parsed_data[0] << 8);
            dht_driver.sensor.data.hum_h = hum / 10;
            dht_driver.sensor.data.hum_l = hum % 10;
            int16_t temp = parsed_data[3];
            temp |= (parsed_data[2] << 8);
            if (temp & DHT_TEMP_NEGATIVE_MASK)
            {
               temp &= ~DHT_TEMP_NEGATIVE_MASK;
               temp = -temp;
            }
            dht_driver.sensor.data.temp_h = temp / 10;
            dht_driver.sensor.data.temp_l = temp % 10;
            break;
         }
         default:
         {
            dht_driver.sensor.data.hum_h = parsed_data [0];
            dht_driver.sensor.data.hum_l = parsed_data [1];
            dht_driver.sensor.data.temp_h = parsed_data [2];
            dht_driver.sensor.data.temp_l = parsed_data [3];
            break;
         }
      }

      logger_send(LOG_ERROR, __func__, "got data %d %d %d %d %d", parsed_data[0], parsed_data[1], parsed_data[2], parsed_data[3], parsed_data[4]);
      result =  RETURN_OK;
   }
   else
   {
      logger_send(LOG_ERROR, __func__, "invalid cs: r %d c %d", parsed_data[4], checksum);
      result = RETURN_NOK;
   }
   return result;
}

DHT_SENSOR_TYPE dht_get_sensor_type(const uint8_t* data)
{
   DHT_SENSOR_TYPE result = DHT_TYPE_DHT11;
   int16_t hum = data[1];
   hum |= (data[0] << 8);
   if (hum <= 1000)
   {
      result = DHT_TYPE_DHT22;
   }
   return result;
}

void dht_handle_exti_interrupt(uint32_t pr_mask)
{
   /* handle interrupt only from currently selected device */
   if (pr_mask == DHT_REG_MAP[dht_driver.sensor.id].exti_pr_mask)
   {
      dht_handle_timestamp();
   }
}

void dht_handle_timestamp()
{
   GPIOB->ODR ^= GPIO_ODR_ODR_9;//TODO to remove
   DHT_MEASURE_TIMESTAMPS[dht_timestamp_idx] = TIM2->CNT;
   dht_timestamp_idx++;
   if (dht_timestamp_idx == DHT_TIMESTAMPS_BUFFER_SIZE)
   {
      sch_set_task_state(&dht_on_timeout, TASKSTATE_STOPPED); // stop timeout counter
      TIM2->CR1 &= ~TIM_CR1_CEN;
      dht_measurement_ready = 1;
      dht_driver.state = DHT_STATE_DATA_DECODING;
   }
   TIM2->CNT = 0;
}

void EXTI15_10_IRQHandler()
{
   if (EXTI->PR & DHT_REG_MAP[dht_driver.sensor.id].exti_pr_mask)
   {
      EXTI->PR = DHT_REG_MAP[dht_driver.sensor.id].exti_pr_mask;
      if (dht_driver.state == DHT_STATE_READING)
      {
         dht_handle_exti_interrupt(DHT_REG_MAP[dht_driver.sensor.id].exti_pr_mask);
      }
   }
   else
   {
      /* clear all interrupts */

   }
}

void EXTI9_5_IRQHandler()
{
   if (EXTI->PR & DHT_REG_MAP[dht_driver.sensor.id].exti_pr_mask)
   {
      EXTI->PR = DHT_REG_MAP[dht_driver.sensor.id].exti_pr_mask;
      if (dht_driver.state == DHT_STATE_READING)
      {
         dht_handle_exti_interrupt(DHT_REG_MAP[dht_driver.sensor.id].exti_pr_mask);
      }
   }
   else
   {
      /* clear all interrupts */

   }
}
