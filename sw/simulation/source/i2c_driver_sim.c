/* =============================
 *   Includes of common headers
 * =============================*/
/* =============================
 *  Includes of project headers
 * =============================*/
#include "i2c_driver.h"
#include "Logger.h"
#include "hw_stub.h"
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
RET_CODE i2c_validate_timeout(uint16_t timeout);
void i2c_print_buffer(I2C_OP_TYPE type);
I2C_STATUS i2c_get_status();
/* =============================
 *       Internal types
 * =============================*/
typedef struct I2C_DRIVER_TYPE
{
   I2C_ADDRESS address;
   I2C_STATUS status;
   I2C_OP_TYPE type;
   uint16_t timeout;
   uint8_t transaction_ready;
   uint16_t bytes_handled;
} I2C_DRIVER_TYPE;
/* =============================
 *      Module variables
 * =============================*/
uint8_t I2C_DRV_BUF[I2C_DRV_BUFFER_SIZE];
volatile I2C_DRIVER_TYPE i2c_driver;
I2C_CALLBACK i2c_drv_callback;

RET_CODE i2c_initialize()
{
   logger_send(LOG_SIM, __func__, "");
   i2c_driver.address = 0;
   i2c_driver.timeout = I2C_DEFAULT_TIMEOUT_MS;
   i2c_driver.transaction_ready = 0;
   i2c_reset();
   return RETURN_OK;
}
RET_CODE i2c_write_async(I2C_ADDRESS address, const uint8_t* data, uint8_t size, I2C_CALLBACK callback)
{
   logger_send(LOG_SIM, __func__, "writing %d bytes to 0x%x",size, address);
   i2c_drv_callback = callback;
   hwstub_i2c_write(address, data, size);
   i2c_driver.bytes_handled = size;
   i2c_driver.type = I2C_OP_WRITE;
   i2c_driver.transaction_ready = 1;
   return RETURN_OK;
}
I2C_STATUS i2c_write(I2C_ADDRESS address, const uint8_t* data, uint8_t size)
{
   logger_send(LOG_SIM, __func__, "writing %d bytes to 0x%x",size, address);
   I2C_STATUS result = I2C_STATUS_ERROR;
   if (data)
   {
      RET_CODE write_result = hwstub_i2c_write(address, data, size);
      result = write_result == RETURN_OK? I2C_STATUS_OK : I2C_STATUS_ERROR;
   }
   return result;
}
RET_CODE i2c_read_async(I2C_ADDRESS address, uint8_t size, I2C_CALLBACK callback)
{
   logger_send(LOG_SIM, __func__, "reading %d bytes from 0x%x",size, address);
   i2c_drv_callback = callback;
   RET_CODE res = hwstub_i2c_read(address, I2C_DRV_BUF, size);
   if (res == RETURN_OK)
   {
      i2c_driver.bytes_handled = size;
      i2c_driver.type = I2C_OP_READ;
      i2c_driver.transaction_ready = 1;
   }
   return res;
}
I2C_STATUS i2c_read(I2C_ADDRESS address, uint8_t* data, uint8_t size)
{
   logger_send(LOG_SIM, __func__, "reading %d bytes from 0x%x",size, address);
   I2C_STATUS result = I2C_STATUS_ERROR;
   if (data)
   {
      RET_CODE res = hwstub_i2c_read(address, data, size);
      if (res == RETURN_OK)
      {
         result = I2C_STATUS_OK;
      }
   }
   return result;
}

void i2c_print_buffer(I2C_OP_TYPE type)
{
   if (logger_get_group_state(LOG_SIM) == LOGGER_GROUP_ENABLE)
   {
      char data_dump [80];
      uint8_t idx = 0;
      for (uint8_t i = 0; i < i2c_driver.bytes_handled; i++)
      {
         idx += string_format(&data_dump[idx], "%x ", I2C_DRV_BUF[i]);
      }
      logger_send(LOG_SIM, __func__, "%s:%s", type == I2C_OP_WRITE? "WRITE" : "READ", data_dump);
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
      result = RETURN_OK;
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
}
void i2c_deinitialize()
{
}

I2C_STATUS i2c_get_status()
{
   return I2C_STATUS_OK;
}

void i2c_watcher()
{
   if (i2c_driver.transaction_ready)
   {
      logger_send(LOG_SIM, __func__, "transaction ready, calling callbacks");
      i2c_driver.transaction_ready = 0;
      i2c_print_buffer(i2c_driver.type);
      if (i2c_drv_callback)
      {
         i2c_drv_callback(i2c_driver.type, I2C_STATUS_OK, I2C_DRV_BUF, i2c_driver.bytes_handled);
      }
   }
}
