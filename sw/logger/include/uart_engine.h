#ifndef _UART_ENGINE_H
#define _UART_ENGINE_H

#include "return_codes.h"

/*
 * This module provides core implementation of UART communication.
 * Allows to read/write data from/to UART peripherial
 * Communication is made in string form.
 *
 * String delimiter char "\n"
 *
 *
*/

typedef struct
{
	uint32_t baudrate;
	char delimiter;
	uint16_t buffer_size;
}UART_Config;

RET_CODE uartengine_initialize(UART_Config*);
void uartengine_deinitialize();
RET_CODE uartengine_send_string(const char *);
const char * uartengine_get_string();
RET_CODE uartengine_is_string();


#endif
