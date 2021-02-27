/* =============================
 *   Includes of common headers
 * =============================*/
#include <string.h>
#include <stdlib.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "wifi_driver.h"
#include "hw_stub.h"
#include "Logger.h"
/* =============================
 *          Defines
 * =============================*/
/* =============================
 *   Internal module functions
 * =============================*/
void wifi_call_client_callback(ClientEvent ev, ServerClientID id, const char* data);
void wifi_on_client_event(ClientEvent ev, ServerClientID id, const char* data);
/* =============================
 *      Module variables
 * =============================*/
void(*wifi_status_callback)(ClientEvent ev, ServerClientID id, const char* data);



RET_CODE wifi_initialize(const WIFI_UART_Config* config)
{
   hwstub_wifi_register_device_event_listener(&wifi_on_client_event);
   logger_send(LOG_SIM, __func__, "");
   return RETURN_OK;
}

void wifi_deinitialize()
{
}

void wifi_on_client_event(ClientEvent ev, ServerClientID id, const char* data)
{
   logger_send(LOG_SIM, __func__, "got ev %u, id %u, data: %s", ev, id, data? data : "(NULL)");
   wifi_call_client_callback(ev, id, data);
}

RET_CODE wifi_register_client_event_callback(void(*callback)(ClientEvent ev, ServerClientID id, const char* data))
{
   RET_CODE result = RETURN_NOK;
   if (!wifi_status_callback)
   {
      wifi_status_callback = callback;
      result = RETURN_OK;
   }
   return result;
}

void wifi_unregister_client_event_callback()
{
   wifi_status_callback = NULL;
}

void wifi_call_client_callback(ClientEvent ev, ServerClientID id, const char* data)
{
   if (wifi_status_callback)
   {
      wifi_status_callback(ev, id, data);
   }
}
void wifi_data_watcher()
{

}

RET_CODE wifi_test()
{
   return RETURN_OK;
}

RET_CODE wifi_reset()
{
   return RETURN_OK;
}

RET_CODE wifi_disable_echo()
{
   return RETURN_OK;
}

RET_CODE wifi_send_data(ServerClientID id, const char* data, uint16_t size)
{
   logger_send(LOG_SIM, __func__, "sending to id %u:%s", id, data? data : "(NULL)");
   return hwstub_wifi_send_data(id, data, size);
}

RET_CODE wifi_send_bytes(ServerClientID id, const uint8_t* data, uint16_t size)
{
   logger_send(LOG_SIM, __func__, "sending %u bytes to id %u", size, id);
   return hwstub_wifi_send_bytes(id, data, size);
}

RET_CODE wifi_connect_to_network(const char* ssid, const char* password)
{
   return RETURN_OK;
}

RET_CODE wifi_disconnect_from_network()
{
   return RETURN_OK;
}

RET_CODE wifi_set_mac_address(const char* mac)
{
   return RETURN_OK;
}

RET_CODE wifi_open_server(uint16_t port)
{
   return RETURN_OK;
}
RET_CODE wifi_close_server()
{
   return RETURN_OK;
}

RET_CODE wifi_allow_multiple_clients(uint8_t state)
{
   return RETURN_OK;
}

RET_CODE wifi_connect_server(ConnType type, const char* server, uint16_t port)
{
   return RETURN_OK;
}

RET_CODE wifi_disconnect_server()
{
   return RETURN_OK;
}

RET_CODE wifi_get_time(const char* ntp_server, TimeItem* item)
{
   hwstub_wifi_get_time(ntp_server, item);
   return RETURN_OK;
}

RET_CODE wifi_set_ip_address(IPAddress* ip_address)
{
   return RETURN_OK;
}

RET_CODE wifi_get_ip_address(IPAddress* ip_address)
{
   hwstub_wifi_get_ip_address(ip_address);
   return RETURN_OK;
}
RET_CODE wifi_get_current_network_name(char* buffer, uint8_t size)
{
   hwstub_wifi_get_current_network_name(buffer, size);
   return RETURN_OK;

}
RET_CODE wifi_request_client_details(ClientID* client)
{
   hwstub_wifi_request_client_details(client);
   return RETURN_OK;
}
