/* =============================
 *   Includes of common headers
 * =============================*/
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "command_parser.h"
#include "string_formatter.h"
#include "inputs_board.h"
#include "relays_board.h"
#include "i2c_driver.h"
#include "dht_driver.h"
#include "bathroom_fan.h"
#include "env_monitor.h"
#include "Logger.h"
#include "stairs_led_module.h"
#include "system_config_values.h"
#include "stm32f4xx.h"
/* =============================
 *          Defines
 * =============================*/
#define CMD_REPLY_BUFFER_SIZE 512
#define CMD_PARSER_USE_ECHO 1
/* =============================
 *   Internal module functions
 * =============================*/
void cmd_prepare_response(RET_CODE result);
void cmd_send_response();
RET_CODE cmd_parse_data(const char* data);
RET_CODE cmd_handle_wifimgr_subcommand(const char** command, uint8_t size);
RET_CODE cmd_handle_slm_subcommand(const char** command, uint8_t size);
RET_CODE cmd_handle_inp_subcommand(const char** command, uint8_t size);
RET_CODE cmd_handle_rel_subcommand(const char** command, uint8_t size);
RET_CODE cmd_handle_i2cdrv_subcommand(const char** command, uint8_t size);
RET_CODE cmd_handle_dhtdrv_subcommand(const char** command, uint8_t size);
RET_CODE cmd_handle_bathfan_subcommand(const char** command, uint8_t size);
RET_CODE cmd_handle_env_subcommand(const char** command, uint8_t size);
RET_CODE cmd_handle_logger_subcommand(const char** command, uint8_t size);
RET_CODE cmd_handle_system_subcommand(const char** command, uint8_t size);
/* =============================
 *      Module variables
 * =============================*/
char CMD_REPLY_BUFFER [CMD_REPLY_BUFFER_SIZE];
RET_CODE(*COMMAND_RESPOND)(const char* data);


void cmd_register_sender(RET_CODE(*callback)(const char* data))
{
   COMMAND_RESPOND = callback;
}
void cmd_unregister_sender()
{
   COMMAND_RESPOND = NULL;
}
void cmd_prepare_response(RET_CODE result)
{
   string_format(CMD_REPLY_BUFFER, "%s", result == RETURN_OK? "OK\n" : "ERROR\n");
}

void cmd_send_response()
{
   if (COMMAND_RESPOND)
   {
      COMMAND_RESPOND(CMD_REPLY_BUFFER);
   }
#ifdef UNIT_TESTS
   printf("%s\n", CMD_REPLY_BUFFER);
#endif
}
void cmd_handle_data(const char* data)
{
	if (CMD_PARSER_USE_ECHO)
	{
		/* send echo */
		string_format(CMD_REPLY_BUFFER, "CMD: %s\n", data);
		cmd_send_response();
	}
	/* send response */
	cmd_prepare_response(cmd_parse_data(data));
	cmd_send_response();
}

RET_CODE cmd_parse_data(const char* data)
{
	RET_CODE result = RETURN_NOK;
	char cmd_received [strlen(data)];
	const char* cmd_items [10];
	char delims[3] = " ";
	uint8_t index = 0;
	string_format(cmd_received, "%s", data);
	char* item = strtok(cmd_received, delims);

	while(item)
	{
		cmd_items[index] = item;
		item = strtok(NULL, delims);
		index++;
	}

	if (!strcmp(cmd_items[0], "wifimgr")) result =  cmd_handle_wifimgr_subcommand(cmd_items, index);
	if (!strcmp(cmd_items[0], "slm")) result =  cmd_handle_slm_subcommand(cmd_items, index);
	if (!strcmp(cmd_items[0], "inp")) result =  cmd_handle_inp_subcommand(cmd_items, index);
	if (!strcmp(cmd_items[0], "rel")) result =  cmd_handle_rel_subcommand(cmd_items, index);
	if (!strcmp(cmd_items[0], "i2c")) result =  cmd_handle_i2cdrv_subcommand(cmd_items, index);
	if (!strcmp(cmd_items[0], "dht")) result =  cmd_handle_dhtdrv_subcommand(cmd_items, index);
   if (!strcmp(cmd_items[0], "fan")) result =  cmd_handle_bathfan_subcommand(cmd_items, index);
   if (!strcmp(cmd_items[0], "env")) result =  cmd_handle_env_subcommand(cmd_items, index);
   if (!strcmp(cmd_items[0], "log")) result =  cmd_handle_logger_subcommand(cmd_items, index);
   if (!strcmp(cmd_items[0], "system")) result =  cmd_handle_system_subcommand(cmd_items, index);
	return result;
}

RET_CODE cmd_handle_system_subcommand(const char** command, uint8_t size)
{
   RET_CODE result = RETURN_NOK;
   if (!strcmp(command[1], "reset"))
   {
      NVIC_SystemReset();
      result = RETURN_OK;
   }
   if (!strcmp(command[1], "version"))
   {
	  string_format(CMD_REPLY_BUFFER, "VERSION: %s", SYSTEM_VERSION);
	  cmd_send_response();
      result = RETURN_OK;
   }
   return result;
}
RET_CODE cmd_handle_inp_subcommand(const char** command, uint8_t size)
{
   RET_CODE result = RETURN_NOK;
   if (!strcmp(command[1], "get_all"))
   {
      INPUT_STATUS inputs_state [INPUTS_MAX_INPUT_LINES];
      result = inp_get_all(inputs_state);
      if (result == RETURN_OK)
      {
         int offset = string_format(CMD_REPLY_BUFFER, "INP_STATE: ");
         for (uint8_t i = 0; i < INPUTS_MAX_INPUT_LINES; i++)
         {
            offset += string_format(CMD_REPLY_BUFFER + (offset), "%d:%s ", (i + 1), inputs_state[i].state == INPUT_STATE_ACTIVE? "ON" : "OFF");
         }
         string_format(CMD_REPLY_BUFFER + (offset-1), "\n");
         cmd_send_response();
      }
   }
   else if (!strcmp(command[1], "read_all"))
   {
      INPUT_STATUS inputs_state [INPUTS_MAX_INPUT_LINES];
      result = inp_read_all(inputs_state);
      if (result == RETURN_OK)
      {
         int offset = string_format(CMD_REPLY_BUFFER, "INP_STATE: ");
         for (uint8_t i = 0; i < INPUTS_MAX_INPUT_LINES; i++)
         {
            offset += string_format(CMD_REPLY_BUFFER + (offset), "%d:%s ", (i + 1), inputs_state[i].state == INPUT_STATE_ACTIVE? "ON" : "OFF");
         }
         string_format(CMD_REPLY_BUFFER + (offset -1), "\n");
         cmd_send_response();
      }
   }
   else if (!strcmp(command[1], "get_config"))
   {
      INPUTS_CONFIG cfg;
      result = inp_get_config(&cfg);
      if (result == RETURN_OK)
      {
         int offset = string_format(CMD_REPLY_BUFFER, "INP_CONFIG:\n");
         offset += string_format(CMD_REPLY_BUFFER + (offset) , "address: %d\n", cfg.address);
         offset += string_format(CMD_REPLY_BUFFER + (offset) , "item:input no\n");
         for (uint8_t i = 0; i < INPUTS_MAX_INPUT_LINES; i++)
         {
            offset += string_format(CMD_REPLY_BUFFER + (offset), "%d:%d\n", cfg.items[i].item, cfg.items[i].input_no);
         }
         cmd_send_response();
      }
   }
   else if (!strcmp(command[1], "enable_interrupt"))
   {
      inp_enable_interrupt();
      result = RETURN_OK;
   }
   else if (!strcmp(command[1], "disable_interrupt"))
   {
      inp_disable_interrupt();
      result = RETURN_OK;
   }
   else if (!strcmp(command[1], "set_debounce_time"))
   {
      result = inp_set_debounce_time(atoi(command[2]));
   }
   else if (!strcmp(command[1], "get_debounce_time"))
   {
      string_format(CMD_REPLY_BUFFER, "DEB_TIME:%d\n", inp_get_debounce_time());
      cmd_send_response();
      result = RETURN_OK;
   }
   else if (!strcmp(command[1], "set_update_time"))
   {
      result = inp_set_periodic_update_time(atoi(command[2]));
   }
   else if (!strcmp(command[1], "get_update_time"))
   {
      string_format(CMD_REPLY_BUFFER, "UPD_TIME:%d\n", inp_get_periodic_update_time());
      cmd_send_response();
      result = RETURN_OK;
   }
   else if (!strcmp(command[1], "enable_update"))
   {
      inp_enable_periodic_update();
      result = RETURN_OK;
   }
   else if (!strcmp(command[1], "disable_update"))
   {
      inp_disable_periodic_update();
      result = RETURN_OK;
   }

   return result;
}

RET_CODE cmd_handle_rel_subcommand(const char** command, uint8_t size)
{
   RET_CODE result = RETURN_NOK;
   if (!strcmp(command[1], "set"))
   {
      RELAY_ID id = (RELAY_ID)atoi(command[2]);
      RELAY_STATE state = (RELAY_STATE)atoi(command[3]);
      result = rel_set(id, state);
   }
   if (!strcmp(command[1], "get_all"))
   {
      RELAY_STATUS relays_state [RELAYS_BOARD_COUNT];
      result = rel_get_all(relays_state);
      if (result == RETURN_OK)
      {
         int offset = string_format(CMD_REPLY_BUFFER, "REL_STATE: ");
         for (uint8_t i = 0; i < RELAYS_BOARD_COUNT; i++)
         {
            offset += string_format(CMD_REPLY_BUFFER + (offset -1), "%d:%s ", (i + 1), relays_state[i].state == RELAY_STATE_ON? "ON" : "OFF");
         }
         string_format(CMD_REPLY_BUFFER + (offset -2), "\n");
         cmd_send_response();
      }
   }
   else if (!strcmp(command[1], "read_all"))
   {
      RELAY_STATUS relays_state [RELAYS_BOARD_COUNT];
      result = rel_read_all(relays_state);
      if (result == RETURN_OK)
      {
         int offset = string_format(CMD_REPLY_BUFFER, "REL_STATE: ");
         for (uint8_t i = 0; i < RELAYS_BOARD_COUNT; i++)
         {
            offset += string_format(CMD_REPLY_BUFFER + (offset -1), "%d:%s ", (i + 1), relays_state[i].state == RELAY_STATE_ON? "ON" : "OFF");
         }
         string_format(CMD_REPLY_BUFFER + (offset -2), "\n");
         cmd_send_response();
      }
   }
   else if (!strcmp(command[1], "get_config"))
   {
      RELAYS_CONFIG cfg;
      result = rel_get_config(&cfg);
      if (result == RETURN_OK)
      {
         int offset = string_format(CMD_REPLY_BUFFER, "REL_CONFIG:\n");
         offset += string_format(CMD_REPLY_BUFFER + (offset) , "address: %d\n", cfg.address);
         offset += string_format(CMD_REPLY_BUFFER + (offset) , "item:relay no\n");
         for (uint8_t i = 0; i < RELAYS_BOARD_COUNT; i++)
         {
            offset += string_format(CMD_REPLY_BUFFER + (offset), "%d:%d\n", cfg.items[i].id, cfg.items[i].relay_no);
         }
         cmd_send_response();
      }
   }
   else if (!strcmp(command[1], "set_update_time"))
   {
      result = rel_set_verification_period(atoi(command[2]));
   }
   else if (!strcmp(command[1], "get_update_time"))
   {
      string_format(CMD_REPLY_BUFFER, "UPD_TIME:%d\n", rel_get_verification_period());
      cmd_send_response();
      result = RETURN_OK;
   }
   else if (!strcmp(command[1], "enable_update"))
   {
      rel_enable_verification();
      result = RETURN_OK;
   }
   else if (!strcmp(command[1], "disable_update"))
   {
      rel_disable_verification();
      result = RETURN_OK;
   }

   return result;
}


RET_CODE cmd_handle_slm_subcommand(const char** command, uint8_t size)
{
   RET_CODE result = RETURN_NOK;
   if (!strcmp(command[1], "get_config"))
   {
      SLM_CONFIG cfg;
      result = slm_get_config(&cfg);
      string_format(CMD_REPLY_BUFFER, "SLM_CONFIG:\nID:%d\nOFF_MODE:%d\naddr:%d\n", cfg.program_id, cfg.off_effect_mode, cfg.address);
      cmd_send_response();
   }
   else if (!strcmp(command[1], "start"))
   {
      result = slm_start_program();
   }
   else if (!strcmp(command[1], "start_alwon"))
   {
      result = slm_start_program_alw_on();
   }
   else if (!strcmp(command[1], "stop"))
   {
      result = slm_stop_program();
   }
   else if (!strcmp(command[1], "status"))
   {
      string_format(CMD_REPLY_BUFFER, "STATUS:state:%d, id:%d\n", (uint8_t)slm_get_state(), (uint8_t)slm_get_current_program_id());
      cmd_send_response();
      result = RETURN_OK;
   }
   else if (!strcmp(command[1], "set_program"))
   {
      result = slm_set_current_program_id((SLM_PROGRAM_ID)(atoi(command[2]) - 1));
   }
   return result;
}

RET_CODE cmd_handle_wifimgr_subcommand(const char** command, uint8_t size)
{
	RET_CODE result = RETURN_NOK;
	if (!strcmp(command[1], "reset"))
	{
		result = wifimgr_reset();
	}
	else if (!strcmp(command[1], "ip"))
	{
		if(!strcmp(command[2], "set"))
		{
			result = wifimgr_set_ip_address(command[3]);
		}
		else if (!strcmp(command[2], "get"))
		{
			IPAddress item = {};
			result = wifimgr_get_ip_address(&item);
			string_format(CMD_REPLY_BUFFER, "IP:%d.%d.%d.%d\n", item.ip_address[0], item.ip_address[1], item.ip_address[2], item.ip_address[3]);
			cmd_send_response();
		}
	}
	else if (!strcmp(command[1], "network"))
	{
		if(!strcmp(command[2], "set"))
		{
			result = wifimgr_set_network_data(command[3], command[4]);
		}
		else if (!strcmp(command[2], "get"))
		{
			char network_name [100];
			result = wifimgr_get_network_name(network_name, 100);
			string_format(CMD_REPLY_BUFFER, "SSID:%s\n", network_name);
			cmd_send_response();
		}
	}
	else if (!strcmp(command[1], "ntpserver"))
	{
		if(!strcmp(command[2], "set"))
		{
			result = wifimgr_set_ntp_server(command[3]);
		}
		else if (!strcmp(command[2], "get"))
		{
			const char* server = wifimgr_get_ntp_server();
			result = server? RETURN_OK : RETURN_NOK;
			string_format(CMD_REPLY_BUFFER, "NTP:%s\n", server);
			cmd_send_response();
		}
	}
	else if (!strcmp(command[1], "serverport"))
	{
		if(!strcmp(command[2], "set"))
		{
			result = wifimgr_set_server_port(atoi(command[3]));
		}
		else if (!strcmp(command[2], "get"))
		{
			uint16_t port = wifimgr_get_server_port();
			result = RETURN_OK;
			string_format(CMD_REPLY_BUFFER, "PORT:%d\n", port);
			cmd_send_response();
		}
	}
	else if (!strcmp(command[1], "clients"))
	{
		if(!strcmp(command[2], "get"))
		{
			ClientID buffer[wifi_get_max_clients()];
			uint8_t count = wifimgr_get_clients_details(buffer);
			result = count != 0? RETURN_OK : RETURN_NOK;
			for (uint8_t i = 0; i < count; i++)
			{
				string_format(CMD_REPLY_BUFFER, "CLIENT%d: type %d, %d.%d.%d.%d\n", buffer[i].id,
											buffer[i].type, buffer[i].address.ip_address[0],
															buffer[i].address.ip_address[1],
															buffer[i].address.ip_address[2],
															buffer[i].address.ip_address[3]);
				cmd_send_response();
			}

		}
	}
	return result;
}

RET_CODE cmd_handle_i2cdrv_subcommand(const char** command, uint8_t size)
{
   RET_CODE result = RETURN_NOK;
   if (!strcmp(command[1], "set_timeout"))
   {
      result = i2c_set_timeout(atoi(command[2]));
   }
   else if (!strcmp(command[1], "get_timeout"))
   {
      string_format(CMD_REPLY_BUFFER, "TIMEOUT:%d\n", i2c_get_timeout());
      cmd_send_response();
      result = RETURN_OK;
   }
   else if (!strcmp(command[1], "read_data"))
   {
      uint8_t address = atoi(command[2]);
      uint8_t size = atoi(command[3]);
      uint8_t buffer [size];
      if (i2c_read(address, buffer, size) == I2C_STATUS_OK)
      {
         int offset = string_format(CMD_REPLY_BUFFER, "DATA:");
         for (uint8_t i = 0; i < size; i++)
         {
            offset += string_format(CMD_REPLY_BUFFER + (offset), " %d", buffer[i]);
         }
         string_format(CMD_REPLY_BUFFER + (offset), "\n");
         cmd_send_response();
         result = RETURN_OK;
      }
   }
   else if (!strcmp(command[1], "write_data"))
   {
      uint8_t address = atoi(command[2]);
      uint8_t size = atoi(command[3]);
      uint8_t buffer [size];
      for (uint8_t i = 0; i < size; i++)
      {
         buffer[i] = atoi(command[i + 4]);
      }

      if (i2c_write(address, buffer, size) == I2C_STATUS_OK)
      {
         result = RETURN_OK;
      }
   }
   else if (!strcmp(command[1], "reset"))
   {
      i2c_reset();
      result = RETURN_OK;
   }

   return result;
}

RET_CODE cmd_handle_dhtdrv_subcommand(const char** command, uint8_t size)
{
   RET_CODE result = RETURN_NOK;
   if (!strcmp(command[1], "set_timeout"))
   {
      result = dht_set_timeout(atoi(command[2]));
   }
   else if (!strcmp(command[1], "get_timeout"))
   {
      string_format(CMD_REPLY_BUFFER, "TIMEOUT:%d\n", dht_get_timeout());
      cmd_send_response();
      result = RETURN_OK;
   }

   else if (!strcmp(command[1], "read_sensor"))
   {
      DHT_SENSOR sensor;
      dht_read((DHT_SENSOR_ID)(atoi(command[2])), &sensor);
      string_format(CMD_REPLY_BUFFER, "DATA:%d.%dC %d.%d%% t:%d\n", sensor.data.temp_h, sensor.data.temp_l, sensor.data.hum_h, sensor.data.hum_l, sensor.type);
      cmd_send_response();
      result = RETURN_OK;
   }
   return result;
}

RET_CODE cmd_handle_bathfan_subcommand(const char** command, uint8_t size)
{
   RET_CODE result = RETURN_NOK;
   if (!strcmp(command[1], "start"))
   {
      result = fan_start();
   }
   else if(!strcmp(command[1], "stop"))
   {
      result = fan_stop();
   }
   else if(!strcmp(command[1], "set_max_work_time"))
   {
      result = fan_set_max_working_time(atoi(command[2]));
   }
   else if(!strcmp(command[1], "set_min_work_time"))
   {
      result = fan_set_min_working_time(atoi(command[2]));
   }
   else if(!strcmp(command[1], "set_hum_thr"))
   {
      result = fan_set_humidity_threshold(atoi(command[2]));
   }
   else if(!strcmp(command[1], "set_thr_hyst"))
   {
      result = fan_set_threshold_hysteresis(atoi(command[2]));
   }
   else if(!strcmp(command[1], "get_state"))
   {
      string_format(CMD_REPLY_BUFFER, "STATE:%d\n", (uint8_t)fan_get_state());
      cmd_send_response();
      result = RETURN_OK;
   }
   else if(!strcmp(command[1], "get_config"))
   {
      FAN_CONFIG cfg;
      result = fan_get_config(&cfg);
      string_format(CMD_REPLY_BUFFER, "FAN_CONFIG:\nMIN_WORK_TIME:%d\nMAX_WORK_TIME:%d\nHUM_THR:%d\nTHR_HYST:%d\n",
                                       cfg.min_working_time_s, cfg.max_working_time_s, cfg.fan_humidity_threshold, cfg.fan_threshold_hysteresis);
      cmd_send_response();
   }
   return result;
}

RET_CODE cmd_handle_env_subcommand(const char** command, uint8_t size)
{
   RET_CODE result = RETURN_NOK;
   if (!strcmp(command[1], "read_sensor"))
   {
      DHT_SENSOR sensor;
      result = env_read_sensor((ENV_ITEM_ID)atoi(command[2]), &sensor);
      string_format(CMD_REPLY_BUFFER, "DATA:%d.%dC %d.%d%% t:%d\n", sensor.data.temp_h, sensor.data.temp_l, sensor.data.hum_h, sensor.data.hum_l, sensor.type);
      cmd_send_response();
   }
   else if(!strcmp(command[1], "read_error"))
   {
      ENV_ERROR_RATE sensor = env_get_error_stats((ENV_ITEM_ID)atoi(command[2]));
      string_format(CMD_REPLY_BUFFER, "DATA:\nNR:%d%% CS:%d%%\n", sensor.nr_err_rate, sensor.cs_err_rate);
      cmd_send_response();
      result = RETURN_OK;
   }
   else if(!strcmp(command[1], "set_meas_period"))
   {
      result = env_set_measurement_period(atoi(command[2]));
   }
   else if(!strcmp(command[1], "get_meas_period"))
   {
      string_format(CMD_REPLY_BUFFER, "PERIOD:%d\n", env_get_measurement_period());
      cmd_send_response();
      result = RETURN_OK;
   }
   return result;
}

RET_CODE cmd_handle_logger_subcommand(const char** command, uint8_t size)
{
   RET_CODE result = RETURN_OK;
   if (!strcmp(command[1], "enable"))
   {
      if (!strcmp(command[2], "module"))
      {
         result = logger_enable();
      }
      else if (logger_string_to_group(command[2]) != LOG_ENUM_MAX)
      {
         result = logger_set_group_state(logger_string_to_group(command[2]), LOGGER_GROUP_ENABLE);
      }
   }
   else if(!strcmp(command[1], "disable"))
   {
      if (!strcmp(command[2], "module"))
      {
         logger_disable();
         result = RETURN_OK;
      }
      else if (logger_string_to_group(command[2]) != LOG_ENUM_MAX)
      {
         result = logger_set_group_state(logger_string_to_group(command[2]), LOGGER_GROUP_DISABLE);
      }
   }
   else if(!strcmp(command[1], "groups_state"))
   {
      uint16_t offset = 0;
      for (uint8_t i = 0; i < LOG_ENUM_MAX; i++)
      {
         offset += string_format(CMD_REPLY_BUFFER + (offset), "%s:%d\n", logger_group_to_string((LogGroup)i),
                                                                         logger_get_group_state((LogGroup)i));
      }
      cmd_send_response();
      result = RETURN_OK;
   }
   return result;
}
