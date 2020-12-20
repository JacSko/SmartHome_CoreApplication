/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdlib.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "stm32f4xx.h"
#include "i2c_driver.h"
#include "Logger.h"
#include "task_scheduler.h"
#include "gpio_lib.h"
/* =============================
 *          Defines
 * =============================*/
#define I2C_DRV_BUFFER_SIZE 32
#define I2C_DEFAULT_TIMEOUT_MS 100
#define I2C_MIN_TIMEOUT_MS 10
#define I2C_MAX_TIMEOUT_MS 500
/* =============================
 *   Internal module functions
 * =============================*/
void i2c_on_timeout();
RET_CODE i2c_validate_timeout(uint16_t timeout);
void i2c_print_buffer(I2C_OP_TYPE type);
I2C_STATUS i2c_get_status();
/* =============================
 *       Internal types
 * =============================*/
typedef enum I2C_STATE
{
   I2C_STATE_IDLE,
   I2C_STATE_STARTED,
   I2C_STATE_ADDRESS_WRITE,
   I2C_STATE_DATA_EXCHANGE,
   I2C_STATE_END_OK,
   I2C_STATE_ERROR,
   I2C_STATE_UNKNOWN,
} I2C_STATE;
typedef struct I2C_DRIVER_TYPE
{
   I2C_STATE state;
   I2C_ADDRESS address;
   I2C_OP_TYPE type;
   uint16_t timeout;
   uint8_t bytes_requested;
   uint8_t bytes_handled;
   uint8_t transaction_ready;
} I2C_DRIVER_TYPE;
/* =============================
 *      Module variables
 * =============================*/
uint8_t I2C_DRV_BUF[I2C_DRV_BUFFER_SIZE];
volatile I2C_DRIVER_TYPE i2c_driver;
I2C_CALLBACK i2c_drv_callback;

RET_CODE i2c_initialize()
{
   logger_send(LOG_I2C_DRV, __func__, "");
   RET_CODE result = RETURN_NOK;
   i2c_driver.state = I2C_STATE_UNKNOWN;
   if (sch_subscribe_and_set(&i2c_on_timeout, TASKPRIO_HIGH, I2C_DEFAULT_TIMEOUT_MS,
                                  TASKSTATE_STOPPED, TASKTYPE_TRIGGER) == RETURN_OK)
   {
      i2c_driver.address = 0;
      i2c_driver.type = I2C_OP_UNKNOWN;
      i2c_driver.timeout = I2C_DEFAULT_TIMEOUT_MS;
      i2c_driver.bytes_requested = 0;
      i2c_driver.bytes_handled = 0;
      i2c_driver.transaction_ready = 0;
      i2c_reset();
      result = RETURN_OK;
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "error");
   return result;
}
void i2c_on_timeout()
{
   if (i2c_driver.state != I2C_STATE_IDLE)
   {
      logger_send(LOG_ERROR,__func__,"timeout");
      i2c_driver.state = I2C_STATE_ERROR;
      i2c_driver.transaction_ready = 1;
   }
}

RET_CODE i2c_write_async(I2C_ADDRESS address, const uint8_t* data, uint8_t size, I2C_CALLBACK callback)
{
   RET_CODE result = RETURN_NOK;
   logger_send(LOG_I2C_DRV, __func__, "writing %u bytes to 0x%x", size, address);
   if (i2c_driver.state == I2C_STATE_IDLE && size <= I2C_DRV_BUFFER_SIZE)
   {
      i2c_driver.address = address;
      i2c_driver.bytes_handled = 0;
      i2c_driver.bytes_requested = size;
      i2c_driver.state = I2C_STATE_STARTED;
      i2c_driver.type = I2C_OP_WRITE;
      i2c_drv_callback = callback;
      for (uint8_t i = 0; i < size; i++)
      {
         I2C_DRV_BUF[i] = data[i];
      }
      I2C1->CR1 |= I2C_CR1_START;
      I2C1->CR1 |= I2C_CR1_ACK;
      sch_trigger_task(&i2c_on_timeout);
      result = RETURN_OK;
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "error");
   return result;
}
I2C_STATUS i2c_write(I2C_ADDRESS address, const uint8_t* data, uint8_t size)
{
   I2C_STATUS result = I2C_STATUS_UNKNOWN;
   if (i2c_write_async(address, data, size, NULL) == RETURN_OK)
   {
      while(i2c_driver.transaction_ready == 0);
      result = i2c_get_status();
      i2c_watcher();
   }
   return result;

}
RET_CODE i2c_read_async(I2C_ADDRESS address, uint8_t size, I2C_CALLBACK callback)
{
   RET_CODE result = RETURN_NOK;
   logger_send(LOG_I2C_DRV, __func__, "reading %u bytes from 0x%x", size, address);
   if (i2c_driver.state == I2C_STATE_IDLE && size <= I2C_DRV_BUFFER_SIZE)
   {
      i2c_driver.address = address;
      i2c_driver.type = I2C_OP_READ;
      i2c_driver.bytes_requested = size;
      i2c_driver.bytes_handled = 0;
      I2C1->CR1 |= I2C_CR1_START;
      I2C1->CR1 |= I2C_CR1_ACK;
      I2C1->CR2 |= I2C_CR2_ITBUFEN;
      i2c_driver.state = I2C_STATE_STARTED;
      i2c_drv_callback = callback;
      sch_trigger_task(&i2c_on_timeout);
      result = RETURN_OK;
   }
   return result;
}
I2C_STATUS i2c_read(I2C_ADDRESS address, uint8_t* data, uint8_t size)
{
   I2C_STATUS result = I2C_STATUS_UNKNOWN;
   if (i2c_read_async(address, size, NULL) == RETURN_OK)
   {
      while(i2c_driver.transaction_ready == 0);
      result = i2c_get_status();
      for (uint8_t i = 0; i < i2c_driver.bytes_handled; i++)
      {
         data[i] = I2C_DRV_BUF[i];
      }
      i2c_watcher();
   }
   return result;
}

void i2c_print_buffer(I2C_OP_TYPE type)
{
   if (logger_get_group_state(LOG_I2C_DRV) == LOGGER_GROUP_ENABLE)
   {
      char data_dump [80];
      uint8_t idx = 0;
      for (uint8_t i = 0; i < i2c_driver.bytes_handled; i++)
      {
         idx += string_format(&data_dump[idx], "%x ", I2C_DRV_BUF[i]);
      }
      logger_send(LOG_I2C_DRV, __func__, "%s:%s", type == I2C_OP_WRITE? "WRITE" : "READ", data_dump);
   }
}

uint16_t i2c_get_timeout()
{
   return i2c_driver.timeout;
}
RET_CODE i2c_set_timeout(uint16_t timeout)
{
   RET_CODE result = RETURN_NOK;
   if (i2c_validate_timeout(timeout) == RETURN_OK)
   {
      logger_send(LOG_I2C_DRV, __func__, "new timeout %d", timeout);
      i2c_driver.timeout = timeout;
      result = sch_set_task_period(&i2c_on_timeout, timeout);
   }
   return result;
}

RET_CODE i2c_validate_timeout(uint16_t timeout)
{
   RET_CODE result = RETURN_NOK;
   if (timeout >= I2C_MIN_TIMEOUT_MS && timeout <= I2C_MAX_TIMEOUT_MS)
   {
      result = RETURN_OK;
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "invalid timeout", timeout);
   return result;
}

void i2c_reset()
{
   logger_send(LOG_ERROR, __func__, "resetting i2c");
   RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
   __DSB();
   RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
   __DSB();
   RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
   __DSB();
   RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
   __DSB();
   I2C1->CR1 |= I2C_CR1_SWRST;
   I2C1->CR1 &= ~I2C_CR1_SWRST;
   gpio_pin_cfg(GPIOB, PB6, gpio_mode_AF4_OD_HS);
   gpio_pin_cfg(GPIOB, PB7, gpio_mode_AF4_OD_HS);
   I2C1->CR2 |= 0x32;   //APB2 clock 50MHz
   I2C1->CR2 |= I2C_CR2_ITEVTEN;
   I2C1->CCR |= 0xFA;   // SCL speed 100kHz (0xB4==180d, how many clock ticks to change state on SCL line)
   I2C1->TRISE |= 0x25; //25
   I2C1->CR2 |= I2C_CR2_ITEVTEN;
   I2C1->CR1 |= I2C_CR1_PE | I2C_CR1_ACK;
   i2c_driver.state = I2C_STATE_IDLE;
   NVIC_EnableIRQ(I2C1_EV_IRQn);
}
void i2c_deinitialize()
{
   i2c_driver.state = I2C_STATE_UNKNOWN;
   i2c_driver.type = I2C_OP_UNKNOWN;
}

I2C_STATUS i2c_get_status()
{
   I2C_STATUS result = i2c_driver.state == I2C_STATE_END_OK? I2C_STATUS_OK : I2C_STATUS_ERROR;
   return result;
}

void i2c_watcher()
{
   if (i2c_driver.transaction_ready)
   {
      i2c_driver.transaction_ready = 0;
      I2C_STATUS status = i2c_get_status();
      if (status == I2C_STATUS_OK)
      {
         logger_send(LOG_I2C_DRV, __func__, "trasnaction OK");
         i2c_print_buffer(i2c_driver.type);
      }
      else
      {
         logger_send(LOG_I2C_DRV, __func__, "error, resetting");
         i2c_reset();
      }
      if (i2c_drv_callback)
      {
         i2c_drv_callback(i2c_driver.type, status, I2C_DRV_BUF, i2c_driver.bytes_handled);
      }
      i2c_driver.state = I2C_STATE_IDLE;
   }
}

void I2C1_EV_IRQHandler (void) {

   if (I2C1->SR1 & I2C_SR1_BTF){
      if(i2c_driver.type == I2C_OP_WRITE && i2c_driver.state != I2C_STATE_IDLE)
      {
         if(i2c_driver.bytes_handled != i2c_driver.bytes_requested)
         {
            I2C1->DR = I2C_DRV_BUF[i2c_driver.bytes_handled++];
         } else {
            I2C1->CR1 |= I2C_CR1_STOP;
            i2c_driver.state = I2C_STATE_END_OK;
            i2c_driver.transaction_ready = 1;
            sch_set_task_state(&i2c_on_timeout, TASKSTATE_STOPPED);
         }
      }
      return;
   }

   if (I2C1->SR1 & I2C_SR1_RXNE)
   {
      I2C_DRV_BUF[i2c_driver.bytes_handled] = I2C1->DR;
      i2c_driver.bytes_handled++;
      if (i2c_driver.bytes_handled == (i2c_driver.bytes_requested -1))
      {
         I2C1->CR1 &= ~I2C_CR1_ACK;
         I2C1->CR1 |= I2C_CR1_STOP;
         return;
      }
      if (i2c_driver.bytes_handled == i2c_driver.bytes_requested)
      {
         i2c_driver.state = I2C_STATE_END_OK;
         i2c_driver.transaction_ready = 1;
         sch_set_task_state(&i2c_on_timeout, TASKSTATE_STOPPED);
         I2C1->CR2 &= ~I2C_CR2_ITBUFEN;
         return;
      }
   }

   if (I2C1->SR1 & I2C_SR1_SB){
      uint8_t Status1 = I2C1->SR1;
      uint8_t Status2 = I2C1->SR2;
      i2c_driver.state = I2C_STATE_ADDRESS_WRITE;
      I2C1->DR = i2c_driver.address;
      (void) Status1;
      (void) Status2;
      return;
   }

   if (I2C1->SR1 & I2C_SR1_ADDR){
      uint8_t Status1;
      uint8_t Status2;
      if (i2c_driver.bytes_requested == 1 && i2c_driver.type == I2C_OP_READ)
      {
         I2C1->CR1 &= ~I2C_CR1_ACK;
         I2C1->CR1 |= I2C_CR1_STOP;
      }
      Status1 = I2C1->SR1;
      Status2 = I2C1->SR2;
      if (i2c_driver.type == I2C_OP_READ)
      {
         i2c_driver.state = I2C_STATE_DATA_EXCHANGE;
      }
      else
      {
         I2C1->DR = I2C_DRV_BUF[i2c_driver.bytes_handled++];
         i2c_driver.state = I2C_STATE_DATA_EXCHANGE;
      }
      (void) Status1;
      (void) Status2;
      return;
   }
}
