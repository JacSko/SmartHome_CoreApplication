#ifndef _BT_ENGINE_H
#define _BT_ENGINE_H

#include "return_codes.h"

/*
 * This module provides core implementation of UART communication.
 * Allows to read/write data from/to UART peripherial
 * Communication is made in string form.
*/

typedef struct
{
	uint32_t baudrate;
	uint16_t buffer_size;
	uint16_t string_size;
}BT_Config;

/**
 * 	Initialisation of btengine.
 */
RET_CODE btengine_initialize(BT_Config*);

/**
 * 	Deinitialisation of btengine.
 */
void btengine_deinitialize();

/**
 * 	Send string over UART.
 * 	Function executed asynchronously.
 * 	String has to be null terminated.
 */
RET_CODE btengine_send_string(const char *);

/**
 *  Returns RETURN_OK if there is at least one string ready to read
 */
RET_CODE btengine_can_read_string();

/**
 * 	Returns pointer to received string if string can be read.
 * 	If there is no string in buffer, NULL is returned.
 */
const char* btengine_get_string();

/**
 * 	Clears all data already received
 */
void btengine_clear_rx();

/**
 * 	Returns amount of bytes currently received.
 * 	It includes all bytes, including special chars like CR, LF.
 */
uint16_t btengine_count_bytes();

/**
 * 	Returns all bytes already received.
 */
const uint8_t* btengine_get_bytes();

/**
 * 	Watcher responsible for calling callbacks - to be called in main program loop.
 */
void btengine_string_watcher();
/**
 * 	Registers callback, which will be called on new received data
 */
RET_CODE btengine_register_callback(void(*callback)(const char *));

/**
 * 	Unregisters previously registered callback
 */
RET_CODE btengine_unregister_callback(void(*callback)(const char *));

#endif
