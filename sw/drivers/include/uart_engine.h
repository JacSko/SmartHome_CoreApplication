#ifndef _UART_ENGINE_H
#define _UART_ENGINE_H
/* ============================= */
/**
 * @file uart_engine.h
 *
 * @brief Module is responsible for handling communcation with ESP module
 *
 * @details
 * Module is handling communication with ESP WiFi device using UART.
 * Allows to register callback function called on new data received.
 * The string_watcher function have to be called in main thread to
 * receive callback notifications.
 *
 * UART1 peripherial:<br>
 * UART_TX = PA2 <br>
 * UART_RX = PA3 <br>
 *
 * @author Jacek Skowronek
 * @date 01/11/2020
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
	uint32_t baudrate;
	uint16_t buffer_size;
	uint16_t string_size;
} UARTEngine_Config;

/**
 * @brief Initialize UART engine module.
 * @param[in] config - Configuration of UART module
 * @return See RETURN_CODES.
 */
RET_CODE uartengine_initialize(const UARTEngine_Config*);
/**
 * @brief Deinitialize UART engine module.
 * @return None
 */
void uartengine_deinitialize();
/**
 * @brief Send string over UART module.
 * @details Function is blocking if there is no place in internal buffer.
 * @param[in] data - pointer to data to send
 * @return See RETURN_CODES.
 */
RET_CODE uartengine_send_string(const char *);
/**
 * @brief Send bytes over UART module.
 * @details Function is blocking if there is no place in internal buffer.
 * @param[in] data - pointer to data to send
 * @param[in] size - size of data
 * @return See RETURN_CODES.
 */
RET_CODE uartengine_send_bytes(const uint8_t* data, uint16_t size);
/**
 * @brief Checks if there is string received in buffer.
 * @return RETURN_OK if string can be read.
 */
RET_CODE uartengine_can_read_string();
/**
 * @brief Get last received string. Before call user should check, if there is string ready to read
 * @return Pointer to string.
 */
const char* uartengine_get_string();
/**
 * @brief Check how bytes is ready to read.
 * @return Bytes count.
 */
uint16_t uartengine_count_bytes();
/**
 * @brief Clear all received (but unread) data
 * @return None.
 */
void uartengine_clear_rx();
/**
 * @brief Get all bytes already received.
 * @details Before call user should check how many bytes are ready.
 * @return Pointer to data bytes.
 */
const uint8_t* uartengine_get_bytes();
/**
 * @brief Constatly check for string in buffer.
 * @details If there is string ready, registered callback are called.
 * @return None.
 */
void uartengine_string_watcher();
/**
 * @brief Register function which will be notified on new string.
 * @param[in] callback - pointer to function.
 * @return See RETURN_CODES.
 */
RET_CODE uartengine_register_callback(void(*callback)(const char *));
/**
 * @brief Unregister callback function.
 * @param[in] callback - pointer to function.
 * @return See RETURN_CODES.
 */
RET_CODE uartengine_unregister_callback(void(*callback)(const char *));

#endif
