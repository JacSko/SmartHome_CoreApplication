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
 *       Internal types
 * =============================*/
typedef struct
{
   uint8_t data_size;
   const uint8_t* data;
} NTF_MESSAGE;
typedef struct
{
   NTF_CMD_ID id;
   NTF_REQ_TYPE type;
   RET_CODE (*process)(const NTF_MESSAGE*);
} NTF_HANDLE_ITEM;
/* =============================
 *   Internal module functions
 * =============================*/
RET_CODE ntfmgr_handle_get_system_time_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_set_system_time_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_get_system_status_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_get_inputs_state_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_get_all_inputs_state_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_get_relays_state_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_get_all_relays_state_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_set_relays_state_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_set_all_relays_state_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_set_fan_state_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_get_fan_state_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_get_slm_state_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_get_slm_program_id_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_set_slm_state_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_set_slm_program_id_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_get_env_sensor_data_cmd(const NTF_MESSAGE* msg);
RET_CODE ntfmgr_handle_get_env_sensor_rate_cmd(const NTF_MESSAGE* msg);


void ntfmgr_prepare_header(NTF_CMD_ID id, NTF_REQ_TYPE req_type, uint8_t data_size);
void ntfmgr_set_message_size(uint8_t size);
void ntfmgr_send_data(uint8_t clientID);

void ntfmgr_on_inputs_change(INPUT_STATUS status);
void ntfmgr_on_relays_change(const RELAY_STATUS* status);
void ntfmgr_on_env_change(ENV_EVENT event, ENV_ITEM_ID id,  const DHT_SENSOR*);
void ntfmgr_on_fan_change(FAN_STATE state);
void ntfmgr_on_slm_change(SLM_STATE state);
//void ntfmgr_process_slm_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args);
//void ntfmgr_process_env_cmd(uint8_t subcmd, NTF_REQ_TYPE req_type, uint8_t data_size, const uint8_t* args);

NTF_HANDLE_ITEM m_handlers[] = {{NTF_SYSTEM_TIME,      NTF_GET, &ntfmgr_handle_get_system_time_cmd},
                                {NTF_SYSTEM_TIME,      NTF_SET, &ntfmgr_handle_set_system_time_cmd},
                                {NTF_SYSTEM_STATUS,    NTF_GET, &ntfmgr_handle_get_system_status_cmd},
                                {NTF_INPUTS_STATE,     NTF_GET, &ntfmgr_handle_get_inputs_state_cmd},
                                {NTF_INPUTS_STATE_ALL, NTF_GET, &ntfmgr_handle_get_all_inputs_state_cmd},
                                {NTF_RELAYS_STATE,     NTF_GET, &ntfmgr_handle_get_relays_state_cmd},
                                {NTF_RELAYS_STATE,     NTF_SET, &ntfmgr_handle_set_relays_state_cmd},
                                {NTF_RELAYS_STATE_ALL, NTF_GET, &ntfmgr_handle_get_all_relays_state_cmd},
                                {NTF_RELAYS_STATE_ALL, NTF_SET, &ntfmgr_handle_set_all_relays_state_cmd},
                                {NTF_FAN_STATE,        NTF_GET, &ntfmgr_handle_get_fan_state_cmd},
                                {NTF_FAN_STATE,        NTF_SET, &ntfmgr_handle_set_fan_state_cmd},
                                {NTF_SLM_STATE,        NTF_SET, &ntfmgr_handle_set_slm_state_cmd},
                                {NTF_SLM_STATE,        NTF_GET, &ntfmgr_handle_get_slm_state_cmd},
                                {NTF_SLM_PROGRAM_ID,   NTF_SET, &ntfmgr_handle_set_slm_program_id_cmd},
                                {NTF_SLM_PROGRAM_ID,   NTF_GET, &ntfmgr_handle_get_slm_program_id_cmd},
                                {NTF_ENV_SENSOR_DATA,  NTF_GET, &ntfmgr_handle_get_env_sensor_data_cmd},
                                {NTF_ENV_SENSOR_ERROR, NTF_GET, &ntfmgr_handle_get_env_sensor_rate_cmd}};

uint8_t m_handlers_size;
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
   m_handlers_size = sizeof(m_handlers) / sizeof(m_handlers[0]);

   if (wifimgr_register_data_callback(&ntfmgr_parse_request) == RETURN_OK)
   {
      result = inp_add_input_listener(&ntfmgr_on_inputs_change);
      logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "cannot subscribe for inputs");

      for (uint8_t i = 1; i <= ENV_SENSORS_COUNT; i++)
      {
         result = env_register_listener(&ntfmgr_on_env_change, (ENV_ITEM_ID)i);
         logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "cannot subscribe for env %d", (uint8_t)i);
      }

      result = rel_add_listener(&ntfmgr_on_relays_change);
      logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "cannot subscribe for relays");

      result = fan_add_listener(&ntfmgr_on_fan_change);
      logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "cannot subscribe for fan");

      result = slm_add_listener(&ntfmgr_on_slm_change);
      logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "cannot subscribe for SLM state");
   }
   logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "cannot init ntfmgr");
   return result;
}
void ntfmgr_parse_request(ServerClientID id, const char* data)
{
   m_bytes_count = 0;
   NTF_CMD_ID cmd_id = (NTF_CMD_ID) data[0];
   NTF_REQ_TYPE cmd_type = (NTF_REQ_TYPE) data[1];
   uint8_t data_size = data[2];

   for (uint8_t i = 0; i < m_handlers_size; i++)
   {
      if (m_handlers[i].id == cmd_id && m_handlers[i].type == cmd_type && m_handlers[i].process)
      {
         NTF_MESSAGE msg = {data_size, (uint8_t*)data + NTF_HEADER_SIZE};
         if (m_handlers[i].process(&msg) != RETURN_OK)
         {
            ntfmgr_prepare_header(cmd_id, cmd_type, 1);
            m_buffer[m_bytes_count++] = (uint8_t) NTF_REPLY_NOK;
         }
         m_buffer[m_bytes_count++] = NTF_MESSAGE_DELIMITER;
         ntfmgr_send_data(id);
      }
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

void ntfmgr_prepare_header(NTF_CMD_ID id, NTF_REQ_TYPE req_type, uint8_t data_size)
{
   m_buffer[NTF_ID_OFFSET] = id;
   m_buffer[NTF_REQ_TYPE_OFFSET] = req_type;
   ntfmgr_set_message_size(data_size);
   m_bytes_count = NTF_HEADER_SIZE;
}
void ntfmgr_set_message_size(uint8_t size)
{
   m_buffer[NTF_BYTES_COUNT_OFFSET] = size;
}

void ntfmgr_on_inputs_change(INPUT_STATUS status)
{
   ntfmgr_prepare_header(NTF_INPUTS_STATE, NTF_NTF, 2);
   m_buffer[m_bytes_count++] = status.id;
   m_buffer[m_bytes_count++] = status.state;
   m_buffer[m_bytes_count++] = NTF_MESSAGE_DELIMITER;
   ntfmgr_send_data(WIFI_BROADCAST_ID);
}
void ntfmgr_on_relays_change(const RELAY_STATUS* status)
{
   ntfmgr_prepare_header(NTF_RELAYS_STATE, NTF_NTF, 2);
   m_buffer[m_bytes_count++] = status->id;
   m_buffer[m_bytes_count++] = status->state;
   m_buffer[m_bytes_count++] = NTF_MESSAGE_DELIMITER;
   ntfmgr_send_data(WIFI_BROADCAST_ID);
}
void ntfmgr_on_env_change(ENV_EVENT event, ENV_ITEM_ID id,  const DHT_SENSOR* sensor)
{
   ntfmgr_prepare_header(event == ENV_EV_ERROR? NTF_ENV_SENSOR_ERROR : NTF_ENV_SENSOR_DATA, NTF_NTF, 6);
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
   ntfmgr_prepare_header(NTF_FAN_STATE, NTF_NTF, 1);
   m_buffer[m_bytes_count++] = (uint8_t) state;
   m_buffer[m_bytes_count++] = NTF_MESSAGE_DELIMITER;
   ntfmgr_send_data(WIFI_BROADCAST_ID);
}
void ntfmgr_on_slm_change(SLM_STATE state)
{
   ntfmgr_prepare_header(NTF_SLM_STATE, NTF_NTF, 1);
   m_buffer[m_bytes_count++] = (uint8_t) state;
   m_buffer[m_bytes_count++] = NTF_MESSAGE_DELIMITER;
   ntfmgr_send_data(WIFI_BROADCAST_ID);
}

RET_CODE ntfmgr_handle_get_system_time_cmd(const NTF_MESSAGE* msg)
{
   RET_CODE result = RETURN_NOK;
   TimeItem* item = time_get();
   if (item)
   {
      ntfmgr_prepare_header(NTF_SYSTEM_TIME, NTF_GET, 8);
      m_buffer[m_bytes_count++] = (uint8_t) NTF_REPLY_OK;
      m_buffer[m_bytes_count++] = item->day;
      m_buffer[m_bytes_count++] = item->month;
      m_buffer[m_bytes_count++] = ((item->year) >> 8) & 0xFF;
      m_buffer[m_bytes_count++] = (item->year) & 0xFF;
      m_buffer[m_bytes_count++] = item->hour;
      m_buffer[m_bytes_count++] = item->minute;
      m_buffer[m_bytes_count++] = item->second;
      result = RETURN_OK;
   }
   return result;
}
RET_CODE ntfmgr_handle_set_system_time_cmd(const NTF_MESSAGE* msg)
{
   RET_CODE result = RETURN_NOK;
   if (msg->data_size == 7)
   {
      TimeItem item = {};
      item.day = msg->data[0];
      item.month = msg->data[1];
      item.year = msg->data[2] << 8;
      item.year |= msg->data[3];
      item.hour = msg->data[4];
      item.minute = msg->data[5];
      item.second = msg->data[6];
      result = time_set_utc(&item);
      ntfmgr_prepare_header(NTF_SYSTEM_TIME, NTF_SET, 1);
      m_buffer[m_bytes_count++] = NTF_REPLY_OK;
   }
   return result;
}
RET_CODE ntfmgr_handle_get_system_status_cmd(const NTF_MESSAGE* msg)
{
   RET_CODE result = RETURN_NOK;
   if (msg->data_size == 0)
   {
      //inputs, relays, temperatures
      INPUT_STATUS inp_status [INPUTS_MAX_INPUT_LINES];
      RELAY_STATUS rel_status [RELAYS_BOARD_COUNT];
      result = inp_get_all(inp_status);
      if (result == RETURN_OK)
      {
         result = rel_get_all(rel_status);
         if (result == RETURN_OK)
         {
            ENV_CONFIG env_cfg = {};
            result = env_get_config(&env_cfg);
            if (result == RETURN_OK)
            {
               uint8_t inputs_found = 0;
               uint8_t relays_found = 0;
               ntfmgr_prepare_header(NTF_SYSTEM_STATUS, NTF_GET, 0);
               m_buffer[m_bytes_count++] = (uint8_t)NTF_REPLY_OK;
               m_bytes_count++; /* skip one bytes to write count of inputs bytes at the beginning - it is not known yet */
               for (uint8_t i = 0; i < INPUTS_MAX_INPUT_LINES; i++)
               {
                  if (inp_status[i].id != INPUT_ENUM_COUNT)
                  {
                     m_buffer[m_bytes_count++] = (uint8_t) inp_status[i].id;
                     m_buffer[m_bytes_count++] = (uint8_t) inp_status[i].state;
                     inputs_found++;
                  }
               }
               m_buffer[NTF_HEADER_SIZE + 1] = inputs_found * 2; /* two bytes per one input */
               m_bytes_count++; /* skip one byte for relays bytes count */

               for (uint8_t i = 0; i < RELAYS_BOARD_COUNT; i++)
               {
                  if (rel_status[i].id != RELAY_ID_ENUM_MAX)
                  {
                     m_buffer[m_bytes_count++] = (uint8_t) rel_status[i].id;
                     m_buffer[m_bytes_count++] = (uint8_t) rel_status[i].state;
                     relays_found++;
                  }
               }
               m_buffer[NTF_HEADER_SIZE + 1 + 1 + (inputs_found * 2)] = relays_found * 2; /* two bytes per one input */

               uint8_t env_sensors_count = sizeof(env_cfg.items)/sizeof(env_cfg.items[0]);
               DHT_SENSOR env_results [ENV_SENSORS_COUNT] = {};
               for (uint8_t i = 0; i < ENV_SENSORS_COUNT; i++)
               {
                  env_get_sensor_data(env_cfg.items[i].env_id, &env_results[i]);
               }
               m_buffer[m_bytes_count++] = (uint8_t) ENV_SENSORS_COUNT * 5;
               for (uint8_t i = 0; i < ENV_SENSORS_COUNT; i++)
               {
                  m_buffer[m_bytes_count++] = (uint8_t)env_cfg.items[i].env_id;
                  m_buffer[m_bytes_count++] = env_results[i].data.hum_h;
                  m_buffer[m_bytes_count++] = env_results[i].data.hum_l;
                  m_buffer[m_bytes_count++] = env_results[i].data.temp_h;
                  m_buffer[m_bytes_count++] = env_results[i].data.temp_l;
               }

               ntfmgr_set_message_size(((inputs_found + relays_found)*2) + (ENV_SENSORS_COUNT * 5) + 3 + 1);
            }

         }
      }
   }
   return result;
}
RET_CODE ntfmgr_handle_get_inputs_state_cmd(const NTF_MESSAGE* msg)
{
   RET_CODE result = RETURN_NOK;
   if (msg->data_size == 1)
   {
      INPUT_STATE state = inp_get((INPUT_ID)msg->data[0]);
      ntfmgr_prepare_header(NTF_INPUTS_STATE, NTF_GET, 2);
      m_buffer[m_bytes_count++] = (uint8_t)NTF_REPLY_OK;
      m_buffer[m_bytes_count++] = (uint8_t)state;
      result = RETURN_OK;
   }
   return result;
}
RET_CODE ntfmgr_handle_get_all_inputs_state_cmd(const NTF_MESSAGE* msg)
{
   INPUT_STATUS inp_status [INPUTS_MAX_INPUT_LINES];
   RET_CODE result = inp_get_all(inp_status);
   uint8_t inputs_found = 0;
   if (result == RETURN_OK)
   {
      ntfmgr_prepare_header(NTF_INPUTS_STATE_ALL, NTF_GET, 0);
      m_buffer[m_bytes_count++] = (uint8_t)NTF_REPLY_OK;
      for (uint8_t i = 0; i < INPUTS_MAX_INPUT_LINES; i++)
      {
         if (inp_status[i].id != INPUT_ENUM_COUNT)
         {
            m_buffer[m_bytes_count++] = (uint8_t) inp_status[i].id;
            m_buffer[m_bytes_count++] = (uint8_t) inp_status[i].state;
            inputs_found++;
         }
      }
      ntfmgr_set_message_size(1 + (inputs_found * 2));
   }
   return result;
}

RET_CODE ntfmgr_handle_get_relays_state_cmd(const NTF_MESSAGE* msg)
{
   RET_CODE result = RETURN_NOK;
   if (msg->data_size == 1)
   {
      RELAY_STATE status = rel_get((RELAY_ID)msg->data[0]);
      if (status != RELAY_STATE_ENUM_MAX)
      {
         ntfmgr_prepare_header(NTF_RELAYS_STATE, NTF_GET, 2);
         m_buffer[m_bytes_count++] = (uint8_t)NTF_REPLY_OK;
         m_buffer[m_bytes_count++] = (uint8_t)status;
         result = RETURN_OK;
      }
   }
   return result;
}
RET_CODE ntfmgr_handle_get_all_relays_state_cmd(const NTF_MESSAGE* msg)
{
   RELAY_STATUS rel_status [RELAYS_BOARD_COUNT];
   RET_CODE result = rel_get_all(rel_status);
   uint8_t relays_found = 0;
   if (result == RETURN_OK)
   {
      ntfmgr_prepare_header(NTF_RELAYS_STATE_ALL, NTF_GET, 0);
      m_buffer[m_bytes_count++] = (uint8_t)NTF_REPLY_OK;
      for (uint8_t i = 0; i < RELAYS_BOARD_COUNT; i++)
      {
         if (rel_status[i].id != RELAY_ID_ENUM_MAX)
         {
            m_buffer[m_bytes_count++] = (uint8_t) rel_status[i].id;
            m_buffer[m_bytes_count++] = (uint8_t) rel_status[i].state;
            relays_found++;
         }
      }
      ntfmgr_set_message_size(1 + (relays_found * 2));
   }
   return result;
}
RET_CODE ntfmgr_handle_set_relays_state_cmd(const NTF_MESSAGE* msg)
{
   RET_CODE result = RETURN_NOK;
   if (msg->data_size == 2)
   {
      result = rel_set((RELAY_ID)msg->data[0], (RELAY_STATE)msg->data[1]);
      ntfmgr_prepare_header(NTF_RELAYS_STATE, NTF_SET, 1);
      m_buffer[m_bytes_count++] = (uint8_t)NTF_REPLY_OK;
   }
   return result;
}
RET_CODE ntfmgr_handle_set_all_relays_state_cmd(const NTF_MESSAGE* msg)
{
   RET_CODE result = RETURN_OK;
   if ((msg->data_size%2) != 1)
   {
      for (uint8_t i = 0; i < msg->data_size / 2; i++)
      {
         RELAY_ID id = (RELAY_ID)msg->data[(i*2)];
         RELAY_STATE state = (RELAY_STATE)msg->data[(i*2)+1];
         result = rel_set(id, state);
         if (result != RETURN_OK)
         {
            break;
         }
      }
      if (result == RETURN_OK)
      {
         ntfmgr_prepare_header(NTF_RELAYS_STATE_ALL, NTF_SET, 1);
         m_buffer[m_bytes_count++] = (uint8_t)NTF_REPLY_OK;
      }
   }
   return result;
}
RET_CODE ntfmgr_handle_set_fan_state_cmd(const NTF_MESSAGE* msg)
{
   RET_CODE result = RETURN_NOK;
   if (msg->data_size == 1)
   {
      FAN_STATE state = (FAN_STATE)msg->data[0];
      if (state == FAN_STATE_ON)
      {
         result = fan_start();
      }
      else if (state == FAN_STATE_OFF)
      {
         result = fan_stop();
      }

      if (result == RETURN_OK)
      {
         ntfmgr_prepare_header(NTF_FAN_STATE, NTF_SET, 1);
         m_buffer[m_bytes_count++] = (uint8_t)NTF_REPLY_OK;
      }
   }
   return result;
}
RET_CODE ntfmgr_handle_get_fan_state_cmd(const NTF_MESSAGE* msg)
{
   RET_CODE result = RETURN_NOK;
   FAN_STATE state = fan_get_state();
   if (state != FAN_STATE_UNKNOWN)
   {
      ntfmgr_prepare_header(NTF_FAN_STATE, NTF_GET, 2);
      m_buffer[m_bytes_count++] = (uint8_t)NTF_REPLY_OK;
      m_buffer[m_bytes_count++] = (uint8_t)state;
      result = RETURN_OK;
   }
   return result;
}
RET_CODE ntfmgr_handle_get_slm_state_cmd(const NTF_MESSAGE* msg)
{
   SLM_STATE state = slm_get_state();
   ntfmgr_prepare_header(NTF_SLM_STATE, NTF_GET, 2);
   m_buffer[m_bytes_count++] = (uint8_t)NTF_REPLY_OK;
   m_buffer[m_bytes_count++] = (uint8_t)state;
   return RETURN_OK;
}
RET_CODE ntfmgr_handle_get_slm_program_id_cmd(const NTF_MESSAGE* msg)
{
   SLM_PROGRAM_ID id = slm_get_current_program_id();
   ntfmgr_prepare_header(NTF_SLM_PROGRAM_ID, NTF_GET, 2);
   m_buffer[m_bytes_count++] = (uint8_t)NTF_REPLY_OK;
   m_buffer[m_bytes_count++] = (uint8_t)id;
   return RETURN_OK;
}
RET_CODE ntfmgr_handle_set_slm_state_cmd(const NTF_MESSAGE* msg)
{
   RET_CODE result = RETURN_NOK;
   if (msg->data_size == 1)
   {
      SLM_STATE state = (SLM_STATE)msg->data[0];
      if (state == SLM_STATE_OFF)
      {
         result = slm_stop_program();
      }
      else if (state == SLM_STATE_ON)
      {
         result = slm_start_program();
      }

      if (result == RETURN_OK)
      {
         ntfmgr_prepare_header(NTF_SLM_STATE, NTF_SET, 1);
         m_buffer[m_bytes_count++] = (uint8_t)NTF_REPLY_OK;
      }
   }
   return result;
}
RET_CODE ntfmgr_handle_set_slm_program_id_cmd(const NTF_MESSAGE* msg)
{
   RET_CODE result = RETURN_NOK;
   if (msg->data_size == 1)
   {
      SLM_PROGRAM_ID id = (SLM_PROGRAM_ID)msg->data[0];
      result = slm_set_current_program_id(id);
      if (result == RETURN_OK)
      {
         ntfmgr_prepare_header(NTF_SLM_PROGRAM_ID, NTF_SET, 1);
         m_buffer[m_bytes_count++] = (uint8_t)NTF_REPLY_OK;
      }
   }
   return result;
}

RET_CODE ntfmgr_handle_get_env_sensor_data_cmd(const NTF_MESSAGE* msg)
{
   DHT_SENSOR sensor = {};
   RET_CODE result = RETURN_NOK;
   if (msg->data_size == 1)
   {
      result = env_get_sensor_data((ENV_ITEM_ID)msg->data[0], &sensor);
      ntfmgr_prepare_header(NTF_ENV_SENSOR_DATA, NTF_GET, 6);
      if (result == RETURN_OK)
      {
         m_buffer[m_bytes_count++] = (uint8_t) NTF_REPLY_OK;
         m_buffer[m_bytes_count++] = (uint8_t) sensor.type;
         m_buffer[m_bytes_count++] = (uint8_t) sensor.data.hum_h;
         m_buffer[m_bytes_count++] = (uint8_t) sensor.data.hum_l;
         m_buffer[m_bytes_count++] = (uint8_t) sensor.data.temp_h;
         m_buffer[m_bytes_count++] = (uint8_t) sensor.data.temp_l;
      }
   }
   return result;
}
RET_CODE ntfmgr_handle_get_env_sensor_rate_cmd(const NTF_MESSAGE* msg)
{
   RET_CODE result = RETURN_NOK;
   if (msg->data_size == 1)
   {
      ENV_ERROR_RATE rate = env_get_error_stats((ENV_ITEM_ID)msg->data[0]);
      ntfmgr_prepare_header(NTF_ENV_SENSOR_ERROR, NTF_GET, 3);
      m_buffer[m_bytes_count++] = (uint8_t) NTF_REPLY_OK;
      m_buffer[m_bytes_count++] = (uint8_t) rate.cs_err_rate;
      m_buffer[m_bytes_count++] = (uint8_t) rate.nr_err_rate;
      result = RETURN_OK;
   }
   return result;
}
