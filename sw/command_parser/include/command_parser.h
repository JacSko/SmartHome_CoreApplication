#ifndef _COMMAND_PARSER_H_
#define _COMMAND_PARSER_H_

/* ============================= */
/**
 * @file command_parser.h
 *
 * @brief Parsing commands received from BT or WiFi
 *
 * @details
 * Module is responsible for parsing string data received. Allows to control whole system by calling functions from another modules.
 * Module requires registering callback function, where the response is provided.
 * Functions (handle_data) need to be called on new received data.
 *
 * @author Jacek Skowronek
 * @date 13/12/2020
 */
/* ============================= */

/* =============================
 *  Includes of project headers
 * =============================*/
#include "return_codes.h"
#include "wifi_manager.h"

/**
 * @brief Register BT write function to be called on module response
 * @param[in] callback - callback function
 * @return None
 */
void cmd_register_sender(RET_CODE(*callback)(const char* data));
/**
 * @brief Unregister BT write function
 * @return None
 */
void cmd_unregister_sender();
/**
 * @brief Provide data to command parser
 * @param[in] id - the sender ID
 * @param[in] data - received command
 * @return None
 */
void cmd_handle_data(const char* data);



#endif
