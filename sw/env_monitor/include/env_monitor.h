#ifndef ENV_MONITOR_H_
#define ENV_MONITOR_H_

/* ============================= */
/**
 * @file env_monitor.h
 *
 * @brief Module responsible for handling temperature and humidity measurement from multiple sensors
 *
 * @details
 * This module is responsible for gathering temperature and humidity data from outside and all rooms inside.
 * It is done via DHT driver. All devices are read in the loop.
 * Every device has own error rates for checksum and no response error (in percents).
 *
 * @author Jacek Skowronek
 * @date 27/12/2020
 */
/* ============================= */

/* =============================
 *          Defines
 * =============================*/
#define ENV_SENSORS_COUNT 6
/* =============================
 *   Includes of common headers
 * =============================*/

/* =============================
 *  Includes of project headers
 * =============================*/
#include "dht_driver.h"
#include "env_types.h"
/* =============================
 *      Global variables
 * =============================*/

/* =============================
 *       Data structures
 * =============================*/
typedef struct ENV_ERROR_RATE
{
   uint8_t nr_err_rate; /**< Percent of NoResponse errors */
   uint8_t cs_err_rate; /**< Percent of Checksum errors */
} ENV_ERROR_RATE;

typedef struct ENV_DHT_MATCH
{
   ENV_ITEM_ID env_id;  /**< ID of sensor from ENV module */
   DHT_SENSOR_ID dht_id; /**< ID of sensor from DHT driver */
} ENV_DHT_MATCH;

typedef struct ENV_CONFIG
{
   uint8_t measure_running;   /**< Flag to idicate whether loop measurement is running */
   uint8_t max_nr_rate;       /**< Maximum NoResponse error percent */
   uint8_t max_cs_rate;       /**< Maximum Checksym error percent */
   ENV_DHT_MATCH items [ENV_SENSORS_COUNT];  /**< Matchers of ENV IDs to DHT IDs */
} ENV_CONFIG;

typedef void(*ENV_CALLBACK)(ENV_EVENT event, ENV_ITEM_ID id,  const DHT_SENSOR*);

/**
 * @brief Initialize the ENV module.
 * @param[in] config - Configuration of ENV module
 * @return See RETURN_CODES.
 */
RET_CODE env_initialize(const ENV_CONFIG* cfg);
/**
 * @brief Deinitialize the ENV module.
 * @return None.
 */
void env_deinitialize();
/**
 * @brief Read data from defined sensor.
 * @param[in] id - the id of the sensor to be read
 * @param[out] buffer - place, where data will be written
 * @return See RETURN_CODES.
 */
RET_CODE env_read_sensor(ENV_ITEM_ID id, DHT_SENSOR* buffer);
/**
 * @brief RGet data from defined sensor.
 * @param[in] id - the id of the sensor
 * @param[out] buffer - place, where data will be written
 * @return See RETURN_CODES.
 */
RET_CODE env_get_sensor_data(ENV_ITEM_ID id, DHT_SENSOR* buffer);
/**
 * @brief Get current error stats for sensor.
 * @param[in] id - the id of the sensor.
 * @return Error rates data.
 */
ENV_ERROR_RATE env_get_error_stats(ENV_ITEM_ID id);
/**
 * @brief Set time between next sensor reading.
 * @param[in] period - the period between measurements in ms.
 * @return See RETURN_CODES.
 */
RET_CODE env_set_measurement_period(uint16_t period);
/**
 * @brief Get the measurement period.
 * @return Measurement period in ms.
 */
uint16_t env_get_measurement_period();
/**
 * @brief Register sensor data listener.
 * @param[in] callback - pointer to function to be called.
 * @param[in] id - the ID of sensor for which callback should be called
 * @return See RETURN_CODES.
 */
RET_CODE env_register_listener(ENV_CALLBACK callback, ENV_ITEM_ID id);
/**
 * @brief Unregister sensor data listener.
 * @param[in] callback - pointer to function.
 * @param[in] id - the ID of sensor.
 * @return None.
 */
void env_unregister_listener(ENV_CALLBACK callback, ENV_ITEM_ID id);





#endif
