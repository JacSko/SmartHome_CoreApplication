#ifndef DHT_DRIVER_H_
#define DHT_DRIVER_H_

/* ============================= */
/**
 * @file dht_driver.h
 *
 * @brief DHT module is responsible for handling low level communication with DHT devices.
 *
 * @details
 * Supported device types: <br>
 * <b>DHT11</b>
 * <b>DHT22</b><br>
 * The device type is auto-detected.
 * It performs data integrity check by checksum calculation.
 * Module is prepared to handle six DHTXX devices.<br>
 * STM Resources used:<br>
 * SENSOR1 - <b>PB9</b><br>
 * SENSOR2 - <b>PB10</b><br>
 * SENSOR3 - <b>PB12</b><br>
 * SENSOR4 - <b>PB13</b><br>
 * SENSOR5 - <b>PB14</b><br>
 * SENSOR6 - <b>PB15</b><br>
 *
 * @author Jacek Skowronek
 * @date 15/12/2020
 */
/* ============================= */

/* =============================
 *   Includes of common headers
 * =============================*/
#include "stdint.h"
/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"
#include "env_types.h"
/* =============================
 *       Data structures
 * =============================*/

/** Struct with measurement results */
typedef struct DHT_SENSOR_DATA
{
	int8_t temp_h; /**< Temperature - integers */
	int8_t temp_l; /**< Temperature - decimals */
	uint8_t hum_h; /**< Humidity - integers */
	uint8_t hum_l; /**< Humidity - decimals*/
} DHT_SENSOR_DATA;

typedef enum DHT_SENSOR_TYPE
{
	DHT_TYPE_DHT11,
	DHT_TYPE_DHT22
} DHT_SENSOR_TYPE;

/** Struct to represent one OneWire device*/
typedef struct DHT_SENSOR
{
	DHT_SENSOR_ID id;
	DHT_SENSOR_TYPE type;
	DHT_SENSOR_DATA data;
} DHT_SENSOR;
/** Measuement status */
typedef enum DHT_STATUS
{
	DHT_STATUS_OK, 				/**< Measurement performed successfully, data is valid */
	DHT_STATUS_NO_RESPONSE, 	/**< No response from device in defined time */
	DHT_STATUS_CHECKSUM_ERROR, /**< Transmission fault - checksum is incorrect */\
	DHT_STATUS_ERROR,          /**< Common error */
	DHT_STATUS_UNKNOWN,
} DHT_STATUS;

typedef void(*DHT_CALLBACK)(DHT_STATUS, DHT_SENSOR*);

/**
 * @brief Initialize DHT driver.
 * @return See RETURN_CODES.
 */
RET_CODE dht_initialize();
/**
 * @brief Read data from DHTXX device.
 * @details
 * This call is non-blocking, therefore callback have to be passed.
 * This callback will be called after read ends (either success or error).
 * @param[in] id - The ID of the sensor to read.
 * @param[in] callback - The callback to be called.
 * @return See RETURN_CODES
 */
RET_CODE dht_read_async(DHT_SENSOR_ID id, DHT_CALLBACK callback);
/**
 * @brief Read data from DHTXX device.
 * @details
 * This call is blocking. It will block for maximum of timeout period.
 * @param[in] id - The ID of the sensor to read.
 * @param[out] sensor - The place where data will be written.
 * @return Status of the measurement.
 */
DHT_STATUS dht_read(DHT_SENSOR_ID id, DHT_SENSOR* sensor);
/**
 * @brief Set maximum measurement timeout.
 * @param[in] timeout - value of timeout in ms
 * @return See RETURN_CODES.
 */
RET_CODE dht_set_timeout(uint16_t timeout);
/**
 * @brief Get measurement timeout.
 * @return Timeout in ms.
 */
uint16_t dht_get_timeout();
/**
 * @brief Watcher responsible for calling callbacks - should be executed in main thread loop.
 * @return See RETURN_CODES.
 */
void dht_data_watcher();

#endif
