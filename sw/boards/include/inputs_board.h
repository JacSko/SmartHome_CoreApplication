#ifndef INPUTS_BOARD_H
#define INPUTS_BOARD_H

/* ============================= */
/**
 * @file inputs_board.h
 *
 * @brief This module allows to manage AC/DC input board.
 *
 * @details
 * Inputs gives information about state of different devices (mostly light state) in house.
 * It has 8x 12V inputs and 8x 230VAC inputs. Communication is performed over I2C bus.
 * The module has possibility read inputs state periodically, but also interrupt from board can be used.
 *
 * @author Jacek Skowronek
 * @date 20/12/2020
 */
/* ============================= */

/* =============================
 *          Defines
 * =============================*/
#define INPUTS_MAX_INPUT_LINES 16  /**< I2C expander has 16 inputs lines */
/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdint.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"
#include "i2c_driver.h"
/* =============================
 *      Global variables
 * =============================*/

/* =============================
 *       Data structures
 * =============================*/
typedef enum INPUT_STATE
{
   INPUT_STATE_INACTIVE,
   INPUT_STATE_ACTIVE,
} INPUT_STATE;
typedef enum INPUT_ID
{
   INPUT_WARDROBE_AC,  /**< Wardrobe light on the ceiling */
   INPUT_WARDROBE_LED, /**< Wardrobe LED light */
   INPUT_BEDROOM_AC,   /**< Bedroom light on the ceiling */
   INPUT_BATHROOM_AC,  /**< Bathroom light on the ceiling */
   INPUT_BATHROOM_LED, /**< Bathroom LED light */
   INPUT_KITCHEN_AC,   /**< Kitchen light on the ceiling */
   INPUT_KITCHEN_WALL, /**< Kitchen light on the wall */
   INPUT_STAIRS_AC,    /**< Stairs light on the ceiling */
   INPUT_STAIRS_SENSOR,/**< The sensor on staircase */
   INPUT_SOCKETS,      /**< AC sockets power */
   INPUT_ENUM_COUNT,   /**< Enumeration count */
} INPUT_ID;

typedef struct INPUT_ITEM
{
   INPUT_ID item;      /**< Item connected to input */
   uint8_t input_no;   /**< Input number from PCB schematic */
} INPUT_ITEM;

typedef struct INPUT_STATUS
{
   INPUT_ID id;         /**< ID of the input */
   INPUT_STATE state;   /**< Input state */
} INPUT_STATUS;
typedef struct INPUTS_CONFIG
{
   I2C_ADDRESS address; /**< I2C address of the input board */
   INPUT_ITEM items [INPUTS_MAX_INPUT_LINES]; /**< Matchers of inputs IDs to inputs numbers */
} INPUTS_CONFIG;

typedef void(*INPUT_LISTENER)(INPUT_ID type, uint8_t state);


/**
 * @brief Initialize inputs module.
 * @param[in] config - Configuration of INPUT module.
 * @return See RETURN_CODES.
 */
RET_CODE inp_initialize(const INPUTS_CONFIG* config);
/**
 * @brief Deinitialize inputs module.
 * @return None.
 */
void inp_deinitialize();
/**
 * @brief Get state of the input.
 * @param[in] id - ID of the input
 * @return Input state.
 */
INPUT_STATE inp_get(INPUT_ID id);
/**
 * @brief Get state of all inputs.
 * @param[in] buffer - place where input state will be written - it has to have place for at least 16 elements
 * @return See RETURN_CODES.
 */
RET_CODE inp_get_all(INPUT_STATUS* buffer);
/**
 * @brief Get inputs config.
 * @param[in] buffer - place where input config will be written.
 * @return See RETURN_CODES.
 */
RET_CODE inp_get_config(INPUTS_CONFIG* buffer);
/**
 * @brief Enable interrupt handling.
 * @details
 *    New input state is read on each active interrupt.
 * @return None.
 */
void inp_enable_interrupt();
/**
 * @brief Disable interrupt handling.
 * @details
 *    To disable interrupt, the periodic update have to be enabled.
 * @return See None.
 */
void inp_disable_interrupt();
/**
 * @brief Set debounce time.
 * @details
 *    This is the delays in ms between first interrupt fire to read new inputs state.
 * @param[in] time - time in ms.
 * @return See RETURN_CODES.
 */
RET_CODE inp_set_debounce_time(uint16_t time);
/**
 * @brief Get debounce time.
 * @return Debounce time in ms.
 */
uint16_t inp_get_debounce_time();
/**
 * @brief Set period of automatic inputs update
 * @param[in] time - time in ms.
 * @return Debounce time in ms.
 */
RET_CODE inp_set_periodic_update_time(uint16_t period);
/**
 * @brief Get autoupdate period.
 * @return Autoupdate time in ms.
 */
uint16_t inp_get_periodic_update_time();
/**
 * @brief Enable automatic inputs update
 * @return None.
 */
void inp_enable_periodic_update();
/**
 * @brief Disable automatic inputs update
 * @return None.
 */
void inp_disable_periodic_update();
/**
 * @brief Add callback to be called on inputs state change
 * @param[in] callback - Pointer to function to be called.
 * @return See RETURN_CODES.
 */
RET_CODE inp_add_input_listener(INPUT_LISTENER callback);
/**
 * @brief Remove callback.
 * @param[in] callback - Pointer to function.
 * @return None.
 */
void inp_remove_input_listener(INPUT_LISTENER callback);






#endif
