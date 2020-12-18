#ifndef _I2C_DRIVER_H_
#define _I2C_DRIVER_H_

/* ============================= */
/**
 * @file i2c_driver.h
 *
 * @brief I2C module is responsible for handling low level communication over I2C bus.
 *
 * @details
 *
 * @author Jacek Skowronek
 * @date 18/12/2020
 */
/* ============================= */

/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdint.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"
/* =============================
 *       Data structures
 * =============================*/
typedef uint8_t I2C_ADDRESS;
typedef enum I2C_OP_TYPE
{
   I2C_OP_WRITE,   /**< Write data to device */
   I2C_OP_READ,   /**< Reads data from device */
   I2C_OP_UNKNOWN,
} I2C_OP_TYPE;

typedef enum I2C_STATUS
{
   I2C_STATUS_OK,    /** Requested command executed correctly */
   I2C_STATUS_ERROR, /** Command executed with errors */
} I2C_STATUS;

typedef void(*I2C_CALLBACK)(I2C_OP_TYPE, I2C_STATUS, const uint8_t* data, uint8_t size);
/**
 * @brief Initialize I2C driver.
 * @return See RETURN_CODES.
 */
RET_CODE i2c_initialize();
/**
 * @brief Write data on I2C bus.
 * @details
 * This call is non-blocking, therefore callback have to be passed.
 * This callback will be called after write ends (either success or error).
 * @param[in] id - The address of the device.
 * @param[in] data - pointer to data.
 * @param[in] size - size of the data to write (in bytes).
 * @param[in] callback - The callback to be called.
 * @return See RETURN_CODES
 */
RET_CODE i2c_write_async(I2C_ADDRESS address, const uint8_t* data, uint8_t size, I2C_CALLBACK callback);
/**
 * @brief Write data on I2C bus.
 * @details
 * This call is blocking for maximum of timeout value.
 * @param[in] id - The address of the device.
 * @param[in] data - pointer to data.
 * @param[in] size - size of the data to write in bytes
 * @return See I2C_STATUS
 */
I2C_STATUS i2c_write(I2C_ADDRESS address, const uint8_t* data, uint8_t size);
/**
 * @brief Read data over I2C bus.
 * @details
 * This call is non-blocking, therefore callback have to be passed.
 * This callback will be called after read ends (either success or error).
 * Data will be provided from internal driver buffer over callback.
 * @param[in] id - The address of the device.
 * @param[in] size - size of the data to read (in bytes).
 * @param[in] callback - The callback to be called.
 * @return See RETURN_CODES
 */
RET_CODE i2c_read_async(I2C_ADDRESS address, uint8_t size, I2C_CALLBACK callback);
/**
 * @brief Read data on I2C bus.
 * @details
 * This call is blocking for maximum of timeout value.
 * It requires to pass pointer to buffer where data should be written.
 * It is user responsibility to assert correct buffer size.
 * @param[in] id - The address of the device.
 * @param[in] data - pointer to data.
 * @param[in] size - size of the data to read (in bytes).
 * @return See I2C_STATUS
 */
I2C_STATUS i2c_read(I2C_ADDRESS address, uint8_t* data, uint8_t size);
/**
 * @brief Get maximum timeout.
 * @return Timeout in ms.
 */
uint16_t i2c_get_timeout();
/**
 * @brief Set maximum transaction timeout.
 * @param[in] timeout - value of timeout in ms
 * @return See RETURN_CODES.
 */
RET_CODE i2c_set_timeout(uint16_t timeout);
/**
 * @brief Reset I2C driver.
 * @return None.
 */
void i2c_reset();
/**
 * @brief Deinitialize I2C driver.
 * @return None.
 */
void i2c_deinitialize();


#endif
