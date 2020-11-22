#ifndef _UART_ENGINE_H
#define _UART_ENGINE_H

#include "return_codes.h"

/*
 * This module provides core implementation of UART communication.
 * Allows to read/write data from/to UART peripherial
 * Communication is made in string form.
*/

typedef struct
{
	uint32_t baudrate;
	char delimiter;
	uint16_t buffer_size;
	uint16_t string_size;
}UART_Config;

/**
 * 	Initialisation of uartengine.
 */
RET_CODE uartengine_initialize(UART_Config*);

/**
 * 	Deinitialisation of uartengine.
 */
void uartengine_deinitialize();

/**
 * 	Send string over UART.
 * 	Function executed asynchronously.
 * 	String has to be null terminated.
 */
RET_CODE uartengine_send_string(const char *);

/**
 *  Returns RETURN_OK if there is at least one string ready to read
 */
RET_CODE uartengine_can_read_string();

/**
 * 	Returns pointer to received string if string can be read.
 * 	If there is no string in buffer, NULL is returned.
 */
const char* uartengine_get_string();

/**
 * 	Watcher responsible for calling callbacks - to be called in main program loop.
 */
void uartengine_string_watcher();
/**
 * 	Registers callback, which will be called on new received data
 */
RET_CODE uartengine_register_callback(void(*callback)(const char *));

/**
 * 	Unregisters previously registered callback
 */
RET_CODE uartengine_unregister_callback(void(*callback)(const char *));

#endif
