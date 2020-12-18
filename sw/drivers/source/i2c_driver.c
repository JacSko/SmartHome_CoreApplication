/* =============================
 *   Includes of common headers
 * =============================*/

/* =============================
 *  Includes of project headers
 * =============================*/
#include "stm32f4xx.h"
#include "i2c_driver.h"
#include "Logger.h"
/* =============================
 *          Defines
 * =============================*/

/* =============================
 *   Internal module functions
 * =============================*/

/* =============================
 *       Internal types
 * =============================*/

/* =============================
 *      Module variables
 * =============================*/

RET_CODE i2c_initialize()
{

}
RET_CODE i2c_write_async(I2C_ADDRESS address, const uint8_t* data, uint8_t size, I2C_CALLBACK callback)
{

}
RET_CODE i2c_write(I2C_ADDRESS address, const uint8_t* data, uint8_t size)
{

}
RET_CODE i2c_read_async(I2C_ADDRESS address, uint8_t size, I2C_CALLBACK callback)
{

}
I2C_STATUS i2c_read(I2C_ADDRESS address, uint8_t* data, uint8_t size)
{

}
uint16_t i2c_get_timeout()
{

}
RET_CODE i2c_set_timeout(uint16_t timeout)
{

}
void i2c_reset()
{

}
void i2c_deinitialize()
{

}
