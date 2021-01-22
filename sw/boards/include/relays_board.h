#ifndef RELAYS_BOARD_H
#define RELAYS_BOARD_H

/* ============================= */
/**
 * @file relays_board.h
 *
 * @brief This module allows to manage AC/DC relays board.
 *
 * @details
 * Relays allows to control different devices (mostly light state) in house.
 * It has 8x 12V relays and 8x 230VAC relays. Communication is performed over I2C bus.
 * Relays state is periodically checked to detect possible changes (after e.g. power loss).
 *
 * @author Jacek Skowronek
 * @date 22/12/2020
 */
/* ============================= */

/* =============================
 *          Defines
 * =============================*/
#define RELAYS_BOARD_COUNT 16
/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdint.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"
#include "i2c_driver.h"
#include "relays_types.h"
/* =============================
 *      Global variables
 * =============================*/

/* =============================
 *       Data structures
 * =============================*/
typedef struct RELAY_STATUS
{
   RELAY_ID id;         /**< ID of the relay */
   RELAY_STATE state;   /**< State of the relay */
} RELAY_STATUS;

typedef struct RELAY_ITEM
{
   RELAY_ID id;         /**< ID of the relay */
   uint8_t relay_no;    /**< Relay number on board */
} RELAY_ITEM;

typedef struct RELAYS_CONFIG
{
   I2C_ADDRESS address; /**< Board address */
   RELAY_ITEM items [RELAYS_BOARD_COUNT];   /**< Board configuration */
} RELAYS_CONFIG;
/**
 * @brief Initialize relays module.
 * @param[in] address - I2C bus address.
 * @return See RETURN_CODES.
 */
RET_CODE rel_initialize(const RELAYS_CONFIG* config);
/**
 * @brief Deinitialize relays module.
 * @return None.
 */
void rel_deinitialize();
/**
 * @brief Set relay state.
 * @param[in] id - id of the relay.
 * @param[in] state - new state of relay.
 * @return See RETURN_CODES.
 */
RET_CODE rel_set(RELAY_ID id, RELAY_STATE state);
/**
 * @brief Get relay state - state is read from last known relays state.
 * @param[in] id - id of the relay.
 * @return Relay state.
 */
RELAY_STATE rel_get(RELAY_ID id);
/**
 * @brief Get state of all relays - state is read from last known relays state.
 * @param[in] buffer - place where relays status will be written - it have to be at least 16 elements array.
 * @return See RETURN_CODES.
 */
RET_CODE rel_get_all(RELAY_STATUS* buffer);
/**
 * @brief Get state of all relays - state is read from relay board.
 * @param[in] buffer - place where relays status will be written - it have to be at least 16 elements array.
 * @return See RETURN_CODES.
 */
RET_CODE rel_read_all(RELAY_STATUS* buffer);
/**
 * @brief Get relays config.
 * @param[in] buffer - place where relays config will be written.
 * @return See RETURN_CODES.
 */
RET_CODE rel_get_config(RELAYS_CONFIG* buffer);
/**
 * @brief Set period of relays verification.
 * @return See RETURN_CODES.
 */
RET_CODE rel_set_verification_period(uint16_t period);
/**
 * @brief Get period of relays verification.
 * @return Verification period in ms.
 */
uint16_t rel_get_verification_period();
/**
 * @brief Enable relays verification feature
 * @return None.
 */
void rel_enable_verification();
/**
 * @brief Disable relays verification feature
 * @return None.
 */
void rel_disable_verification();
/**
 * @brief Get relay verification state
 * @return See RETURN_CODES.
 */
RET_CODE rel_get_verification_state();


#endif
