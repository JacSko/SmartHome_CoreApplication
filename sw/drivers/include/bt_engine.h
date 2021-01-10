#ifndef _BT_ENGINE_H
#define _BT_ENGINE_H
/* ============================= */
/**
 * @file bt_engine.h
 *
 * @brief Module is responsible for handling communcation with Bluetooth module
 *
 * @details
 * Module is handling communication with Bluetooth device using UART.
 * Allows to register callback function called on new data received.
 * The string_watcher function have to be called in main thread to
 * receive callback notifications.
 *
 * UART1 peripherial:<br>
 * UART_TX = PA9 <br>
 * UART_RX = PA10 <br>
 *
 * @author Jacek Skowronek
 * @date 13/12/2020
 */
/* ============================= */

/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"

/* =============================
 *       Data structures
 * =============================*/
typedef struct
{
	uint32_t baudrate; 	  /**< UART baud rate */
	uint16_t buffer_size; /**< Maximum data buffer size */
	uint16_t string_size; /**< Maximum string size */
}BT_Config;

/**
 * @brief Initialize BT engine module.
 * @param[in] config - Configuration of BT module
 * @return See RETURN_CODES.
 */
RET_CODE btengine_initialize(BT_Config*);
/**
 * @brief Deinitialize BT engine module.
 * @return None
 */
void btengine_deinitialize();
/**
 * @brief Send string over BT module.
 * @details Function is blocking if there is no place in internal buffer.
 * @param[in] data - pointer to data to send
 * @return See RETURN_CODES.
 */
RET_CODE btengine_send_string(const char *);
/**
 * @brief Send bytes over BT module.
 * @details Function is blocking if there is no place in internal buffer.
 * @param[in] data - pointer to data to send
 * @param[in] size - size of data
 * @return See RETURN_CODES.
 */
RET_CODE btengine_send_bytes(const uint8_t*, uint16_t size);
/**
 * @brief Checks if there is string received in buffer.
 * @return RETURN_OK if string can be read.
 */
RET_CODE btengine_can_read_string();
/**
 * @brief Get last received string. Before call user should check, if there is string ready to read
 * @return Pointer to string.
 */
const char* btengine_get_string();
/**
 * @brief Clear all received (but unread) data
 * @return None.
 */
void btengine_clear_rx();
/**
 * @brief Check how bytes is ready to read.
 * @return Bytes count.
 */
uint16_t btengine_count_bytes();
/**
 * @brief Get all bytes already received.
 * @details Before call user should check how many bytes are ready.
 * @return Pointer to data bytes.
 */
const uint8_t* btengine_get_bytes();
/**
 * @brief Constatly check for string in buffer.
 * @details If there is string ready, registered callback are called.
 * @return None.
 */
void btengine_string_watcher();
/**
 * @brief Register function which will be notified on new string.
 * @param[in] callback - pointer to function.
 * @return See RETURN_CODES.
 */
RET_CODE btengine_register_callback(void(*callback)(const char *));
/**
 * @brief Unregister callback function.
 * @param[in] callback - pointer to function.
 * @return See RETURN_CODES.
 */
RET_CODE btengine_unregister_callback(void(*callback)(const char *));

#endif
