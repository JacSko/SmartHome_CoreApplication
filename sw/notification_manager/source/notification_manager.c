/* =============================
 *   Includes of common headers
 * =============================*/
#include "stdlib.h"
/* =============================
 *  Includes of project headers
 * =============================*/
#include "notification_manager.h"
#include "notification_types.h"
#include "wifi_manager.h"
#include "inputs_board.h"
#include "relays_board.h"
#include "env_monitor.h"
#include "bathroom_fan.h"
#include "stairs_led_module.h"
#include "Logger.h"
#include "time_counter.h"
/* =============================
 *          Defines
 * =============================*/
#define WIFI_BROADCAST_ID 0xFF
/* =============================
 *   Internal module functions
 * =============================*/
void ntfmgr_send_data(uint8_t clientID);
void ntfmgr_on_inputs_change(INPUT_STATUS status);
void ntfmgr_on_relays_change(const RELAY_STATUS* status);
void ntfmgr_on_env_change(ENV_EVENT event, ENV_ITEM_ID id,  const DHT_SENSOR*);
void ntfmgr_on_fan_change(FAN_STATE state);
void ntfmgr_on_slm_change(SLM_STATE state);
void ntfmgr_process_system_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args);
void ntfmgr_process_relay_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args);
void ntfmgr_process_inputs_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args);
void ntfmgr_process_fan_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args);
void ntfmgr_process_slm_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args);
void ntfmgr_process_env_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args);
/* =============================
 *       Internal types
 * =============================*/
uint8_t m_buffer [NTF_MAX_MESSAGE_SIZE];
uint16_t m_bytes_count;
/* =============================
 *      Module variables
 * =============================*/

RET_CODE ntfmgr_init()
{
   RET_CODE result = RETURN_NOK;
   for (uint8_t i = 0; i < NTF_MAX_MESSAGE_SIZE; i++)
   {
      m_buffer[i] = 0;
   }

   if (wifimgr_register_data_callback(&ntfmgr_parse_request) == RETURN_OK)
   {
      result = inp_add_input_listener(&ntfmgr_on_inputs_change);
      logger_send_if(result != RETURN_NOK, LOG_ERROR, __func__, "cannot subscribe for inputs");

      for (uint8_t i = 1; i <= ENV_SENSORS_COUNT; i++)
      {
         result = env_register_listener(&ntfmgr_on_env_change, (ENV_ITEM_ID)i);
         logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "cannot subscribe for env %d", (uint8_t)i);
      }

      result = rel_add_listener(&ntfmgr_on_relays_change);
      logger_send_if(result != RETURN_NOK, LOG_ERROR, __func__, "cannot subscribe for relays");

      result = fan_add_listener(&ntfmgr_on_fan_change);
      logger_send_if(result != RETURN_NOK, LOG_ERROR, __func__, "cannot subscribe for fan");

      result = slm_add_listener(&ntfmgr_on_slm_change);
      logger_send_if(result != RETURN_NOK, LOG_ERROR, __func__, "cannot subscribe for SLM state");
   }
   logger_send_if(result != RETURN_NOK, LOG_ERROR, __func__, "cannot init ntfmgr");
   return result;
}
void ntfmgr_parse_request(ServerClientID id, const char* data)
{
   m_bytes_count = 0;

   const uint8_t* request = (uint8_t*) data;
   NTF_GROUP    group = (NTF_GROUP) request[NTF_GROUP_OFFSET];
   uint8_t      subcmd = (uint8_t) request[NTF_GROUP_SUBCMD_OFFSET];
   NTF_REQ_TYPE req_type = (NTF_REQ_TYPE) request[NTF_GROUP_REQ_TYPE_OFFSET];
   uint8_t      data_size = (uint8_t)request[NTF_GROUP_BYTES_COUNT_OFFSET];

   logger_send(LOG_ERROR, __func__, "%u %u %u %u", (uint8_t) group, subcmd, (uint8_t) req_type, data_size);
   switch(group)
   {
   case NTF_GROUP_SYSTEM:
      ntfmgr_process_system_cmd(subcmd, req_type, data_size, request + NTF_HEADER_SIZE);
      break;
   }

   if (m_bytes_count)
   {
      ntfmgr_send_data(id);
   }
}

void ntfmgr_send_data(uint8_t clientID)
{
   if (clientID == WIFI_BROADCAST_ID)
   {
      wifimgr_broadcast_bytes(m_buffer, m_bytes_count);
   }
   else
   {
      wifimgr_send_bytes(clientID, m_buffer, m_bytes_count);
   }
   m_bytes_count = 0;
}

void ntfmgr_prepare_header(NTF_GROUP group, uint8_t subcmd, NTF_REQ_TYPE req_type, NTF_REPLY_TYPE rep_type, uint8_t data_size)
{
   m_buffer[NTF_GROUP_OFFSET] = group;
   m_buffer[NTF_GROUP_SUBCMD_OFFSET] = subcmd;
   m_buffer[NTF_GROUP_REQ_TYPE_OFFSET] = req_type;
   m_buffer[NTF_GROUP_REP_TYPE_OFFSET] = rep_type;
   m_buffer[NTF_GROUP_BYTES_COUNT_OFFSET] = data_size;
   m_bytes_count = NTF_HEADER_SIZE;
}

void ntfmgr_on_inputs_change(INPUT_STATUS status)
{
   ntfmgr_prepare_header(NTF_GROUP_INPUTS, NTF_INPUTS_STATE, NTF_NTF, NTF_REPLY_UNKNOWN, 2);
   m_buffer[m_bytes_count++] = status.id;
   m_buffer[m_bytes_count++] = status.state;
   m_buffer[m_bytes_count++] = NTF_MESSAGE_DELIMITER;
   ntfmgr_send_data(WIFI_BROADCAST_ID);
}
void ntfmgr_on_relays_change(const RELAY_STATUS* status)
{
   ntfmgr_prepare_header(NTF_GROUP_RELAYS, NTF_RELAYS_STATE, NTF_NTF, NTF_REPLY_UNKNOWN, 2);
   m_buffer[m_bytes_count++] = status->id;
   m_buffer[m_bytes_count++] = status->state;
   m_buffer[m_bytes_count++] = NTF_MESSAGE_DELIMITER;
   ntfmgr_send_data(WIFI_BROADCAST_ID);
}
void ntfmgr_on_env_change(ENV_EVENT event, ENV_ITEM_ID id,  const DHT_SENSOR* sensor)
{
   ntfmgr_prepare_header(NTF_GROUP_ENV, event == ENV_EV_ERROR? NTF_ENV_SENSOR_ERROR : NTF_ENV_SENSOR_DATA, NTF_NTF, NTF_REPLY_UNKNOWN, 6);
   m_buffer[m_bytes_count++] = (uint8_t)id;
   m_buffer[m_bytes_count++] = (uint8_t)sensor->type;
   m_buffer[m_bytes_count++] = (uint8_t)sensor->data.hum_h;
   m_buffer[m_bytes_count++] = (uint8_t)sensor->data.hum_l;
   m_buffer[m_bytes_count++] = (uint8_t)sensor->data.temp_h;
   m_buffer[m_bytes_count++] = (uint8_t)sensor->data.temp_l;
   m_buffer[m_bytes_count++] = NTF_MESSAGE_DELIMITER;
   ntfmgr_send_data(WIFI_BROADCAST_ID);
}
void ntfmgr_on_fan_change(FAN_STATE state)
{
   ntfmgr_prepare_header(NTF_GROUP_FAN, NTF_FAN_STATE, NTF_NTF, NTF_REPLY_UNKNOWN, 1);
   m_buffer[m_bytes_count++] = (uint8_t) state;
   m_buffer[m_bytes_count++] = NTF_MESSAGE_DELIMITER;
   ntfmgr_send_data(WIFI_BROADCAST_ID);
}
void ntfmgr_on_slm_change(SLM_STATE state)
{
   ntfmgr_prepare_header(NTF_GROUP_SLM, NTF_SLM_STATE, NTF_NTF, NTF_REPLY_UNKNOWN, 1);
   m_buffer[m_bytes_count++] = (uint8_t) state;
   m_buffer[m_bytes_count++] = NTF_MESSAGE_DELIMITER;
   ntfmgr_send_data(WIFI_BROADCAST_ID);
}

void ntfmgr_process_system_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args)
{
   switch((NTF_SYSTEM_SUBCMDS)subcmd)
   {
   case NTF_SYSTEM_TIME:
      if (req_type == NTF_SET)
      {
         TimeItem item = {};
         item.day = args[0];
         item.month = args[1];
         item.year = args[2] << 8;
         item.year |= args[3];
         item.hour = args[4];
         item.minute = args[5];
         item.second = args[6];

         RET_CODE result = time_set_utc(&item);
         ntfmgr_prepare_header(NTF_GROUP_SYSTEM, subcmd, req_type, result == RETURN_OK? NTF_REPLY_OK : NTF_REPLY_NOK, 0);
         m_buffer[m_bytes_count++] = NTF_MESSAGE_DELIMITER;
      }
      else if (req_type == NTF_GET)
      {
         TimeItem* item = time_get();
         if (!item)
         {
            ntfmgr_prepare_header(NTF_GROUP_SYSTEM, subcmd, req_type, NTF_REPLY_NOK, 0);
            m_buffer[m_bytes_count++] = NTF_MESSAGE_DELIMITER;
         }
         else
         {
            ntfmgr_prepare_header(NTF_GROUP_SYSTEM, subcmd, req_type, NTF_REPLY_OK, 7);
            m_buffer[m_bytes_count++] = item->day;
            m_buffer[m_bytes_count++] = item->month;
            m_buffer[m_bytes_count++] = ((item->year) >> 8) & 0xFF;
            m_buffer[m_bytes_count++] = (item->year) & 0xFF;
            m_buffer[m_bytes_count++] = item->hour;
            m_buffer[m_bytes_count++] = item->minute;
            m_buffer[m_bytes_count++] = item->second;
            m_buffer[m_bytes_count++] = NTF_MESSAGE_DELIMITER;
         }
      }
      break;
   }

}
void ntfmgr_process_relay_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args)
{

}
void ntfmgr_process_inputs_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args)
{

}
void ntfmgr_process_fan_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args)
{

}
void ntfmgr_process_slm_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args)
{

}
void ntfmgr_process_env_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args)
{

}
