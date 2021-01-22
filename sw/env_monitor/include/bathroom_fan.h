#ifndef BATHROOM_FAN_H_
#define BATHROOM_FAN_H_

/* ============================= */
/**
 * @file bathroom_fan.h
 *
 * @brief Module responsible for controlling bathroom fan
 *
 * @details
 * Module is keeping track of bathroom humidity and controls fan depending on current humidity and temperature.
 * Relays module is used to enable/disable fan power supply.
 * Fan is turned on when received humidity is above fan_humidity_threashold.
 * Fan is turned off when received humidity is below fan_humidity_threshold - fan_threshold_hysteresis.
 * When max_working_time_s reached, fan is turned OFF and switched to suspend state until humidity decrease below fan_humidity_threshold - fan_threshold_hysteresis.
 *
 * @author Jacek Skowronek
 * @date 28/12/2020
 */
/* ============================= */

/* =============================
 *          Defines
 * =============================*/

/* =============================
 *   Includes of common headers
 * =============================*/

/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"
#include "fan_types.h"
/* =============================
 *      Global variables
 * =============================*/

/* =============================
 *       Data structures
 * =============================*/
typedef struct FAN_CONFIG
{
   uint16_t min_working_time_s;        /**< Minimum fan working time after start */
   uint16_t max_working_time_s;        /**< Maximum fan working time after start */
   uint8_t fan_humidity_threshold;     /**< Humidity for starting fan */
   uint8_t fan_threshold_hysteresis;   /**< Hysteresis to disable fan */
} FAN_CONFIG;
/**
 * @brief Initialize the module.
 * @param[in] config - Configuration of Fan module
 * @return See RETURN_CODES.
 */
RET_CODE fan_initialize(const FAN_CONFIG* cfg);
/**
 * @brief Deinitialize the module.
 * @return None.
 */
void fan_deinitialize();
/**
 * @brief Starts fan.
 * @details Working time is not controlled after manual start.
 * @return See RETURN_CODES.
 */
RET_CODE fan_start();
/**
 * @brief Stops fan.
 * @return See RETURN_CODES.
 */
RET_CODE fan_stop();
/**
 * @brief Returns current fan state.
 * @return State of the fan.
 */
FAN_STATE fan_get_state();
/**
 * @brief Sets the fan maximum working time.
 * @param[in] time_s - time in seconds.
 * @return See RETURN_CODES.
 */
RET_CODE fan_set_max_working_time(uint16_t time_s);
/**
 * @brief Sets the fan minimum working time.
 * @param[in] time_s - time in seconds.
 * @return See RETURN_CODES.
 */
RET_CODE fan_set_min_working_time(uint16_t time_s);
/**
 * @brief Sets fan start threshold.
 * @param[in] hum_trigger - humidity in percents.
 * @return See RETURN_CODES.
 */
RET_CODE fan_set_humidity_threshold(uint8_t hum_trigger);
/**
 * @brief Sets fan threshold hysteresis.
 * @param[in] hum_trigger - hysteresis in percents.
 * @return See RETURN_CODES.
 */
RET_CODE fan_set_threshold_hysteresis(uint8_t hum_trigger);
/**
 * @brief Get current module config.
 * @param[out] buffer - place where configuration will be written.
 * @return See RETURN_CODES.
 */
RET_CODE fan_get_config(FAN_CONFIG* buffer);



#endif
