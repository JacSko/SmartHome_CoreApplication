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
 * 	Fills up provided buffer with received string.
 * 	Provided buffer has to have enough capacity.
 */
uint8_t uartengine_get_string(char *);

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
