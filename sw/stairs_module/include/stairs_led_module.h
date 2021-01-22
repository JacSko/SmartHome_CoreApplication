#ifndef STAIRS_LED_MODULE_H_
#define STAIRS_LED_MODULE_H_

/* ============================= */
/**
 * @file stairs_led_module.h
 *
 * @brief Module to control LED lights on stairs
 *
 * @details
 * This module allows to start/stop lights on stairs with many programs.
 *
 * @author Jacek Skowronek
 * @date 30/12/2020
 */
/* ============================= */

/* =============================
 *          Defines
 * =============================*/
#define SLM_MAX_PROGRAM_STEPS 16
#define SLM_MAX_PROGRAMS 3
/* =============================
 *   Includes of common headers
 * =============================*/

/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"
#include "i2c_driver.h"
#include "stairs_led_types.h"
/* =============================
 *      Global variables
 * =============================*/

/* =============================
 *       Data structures
 * =============================*/
typedef struct SLM_STEP
{
   uint8_t leds_state;  /**< State of leds for this step - eg 0b10000001 */
   uint16_t period;     /**< Time in ms, how long this step should be active*/
} SLM_STEP;
typedef struct SLM_PROGRAM
{
  SLM_STEP program_steps [SLM_MAX_PROGRAM_STEPS];     /**< Next steps of program */
  uint8_t program_steps_count;                        /**< Number of programs steps */
  SLM_STEP off_effect_steps [SLM_MAX_PROGRAM_STEPS];  /**< Next steps of OFF effect */
  uint8_t off_effect_steps_count;                     /**< Number of OFF effect steps */
} SLM_PROGRAM;

typedef struct SLM_CONFIG
{
   SLM_PROGRAM_ID program_id;           /**< ID of the default program */
   SLM_OFF_EFFECT_MODE off_effect_mode; /**< State of the OFF effect */
   I2C_ADDRESS address;                 /**< I2C address of the LED board */
} SLM_CONFIG;
/**
 * @brief Initialize Stairs LED module.
 * @param[in] config - Configuration of BT module
 * @return See RETURN_CODES.
 */
RET_CODE slm_initialize(const SLM_CONFIG* config);
/**
 * @brief Deinitialize Stairs LED module.
 * @return See RETURN_CODES.
 */
void slm_deinitialize();
/**
 * @brief Get current module configuration.
 * @param[out] buffer - place, where configuration will be written.
 * @return See RETURN_CODES.
 */
RET_CODE slm_get_config(SLM_CONFIG* buffer);
/**
 * @brief Starts current program. Program need to be stopped manually, there is not timeout.
 * @return See RETURN_CODES.
 */
RET_CODE slm_start_program_alw_on();
/**
 * @brief Starts current program. Program is stopped automatically.
 * @return See RETURN_CODES.
 */
RET_CODE slm_start_program();
/**
 * @brief Stops currently enabled program.
 * @return See RETURN_CODES.
 */
RET_CODE slm_stop_program();
/**
 * @brief Get current module state.
 * @return State of the SLM module.
 */
SLM_STATE slm_get_state();
/**
 * @brief Get current program ID.
 * @return ID of the program.
 */
SLM_PROGRAM_ID slm_get_current_program_id();
/**
 * @brief Set current program ID.
 * @param[in] id - ID of the program to set.
 * @return See RETURN_CODES.
 */
RET_CODE slm_set_current_program_id(SLM_PROGRAM_ID id);
/**
 * @brief Get defined program details.
 * @param[in] id - ID of the program to get.
 * @param[out] buffer - place, where program will be written.
 * @return See RETURN_CODES.
 */
RET_CODE slm_get_program_by_id(SLM_PROGRAM_ID id, SLM_PROGRAM* buffer);
/**
 * @brief Replace existing program with the new one.
 * @param[in] id - ID of the program to replace.
 * @param[out] program - new program.
 * @return See RETURN_CODES.
 */
RET_CODE slm_replace_program(SLM_PROGRAM_ID id, const SLM_PROGRAM* program);


#endif
