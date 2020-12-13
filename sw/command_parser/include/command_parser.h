#ifndef _COMMAND_PARSER_H_
#define _COMMAND_PARSER_H_

/* ============================= */
/**
 * @file command_parser.h
 *
 * @brief Parsing commands received from BT or WiFi
 *
 * @details
 * Module is responsible for parsing string data received from BT or WiFi
 * peripherials. Allows to control whole system by calling functions from another modules.
 * Module requires registering 2 callback function, where the response is provided.
 * Two functions (handle_wifi_data and handle_bt_data) need to be called on new received data.
 * The response is provided only for the channel the request came from.
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
void cmd_register_bt_sender(RET_CODE(*callback)(const char* data));
/**
 * @brief Unregister BT write function
 * @return None
 */
void cmd_unregister_bt_sender();
/**
 * @brief Register WiFi write function to be called on module response
 * @param[in] callback - callback function
 * @return None
 */
void cmd_register_wifi_sender(RET_CODE(*callback)(ServerClientID id, const char* data));
/**
 * @brief Unregister WiFi write function
 * @return None
 */
void cmd_unregister_wifi_sender();
/**
 * @brief Provide data to command parser
 * @param[in] id - the sender ID
 * @param[in] data - received command
 * @return None
 */
void cmd_handle_wifi_data(ServerClientID id, const char* data);
/**
 * @brief Provide data to command parser
 * @param[in] id - the sender ID
 * @param[in] data - received command
 * @return None
 */
void cmd_handle_bt_data(const char* data);



#endif
