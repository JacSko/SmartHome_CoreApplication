#include <stdlib.h>
#include "time_counter.h"
#include "wifi_manager.h"
#include "uart_engine.h"
#include "bt_engine.h"
#include "task_scheduler.h"
#include "command_parser.h"
#include "Logger.h"
#include "i2c_driver.h"
#include "inputs_board.h"
#include "relays_board.h"
#include "stairs_led_module.h"
#include "env_monitor.h"
#include "bathroom_fan.h"
#include "system_timestamp.h"
#include "notification_manager.h"
#include "system_config_values.h"
/**
 * 	System config:
 * 	HSI - 16MHz
 * 	PLL_M - 16
 * 	PLL_N - 400
 * 	PLL_P - 4
 * 	PLL_Q - 7
 *
 * 	PLL clock - 100MHz
 *
 * 	HCLK - 100MHz
 * 	PCLK2 - 50MHz
 * 	PCLK1 - 25MHz
 *
 * 	AHB_prescaler - 1 - 100MHz
 *    APB1 prescaler - 2 - 50MHz
 *    APB2 prescaler - 1 - 100MHz
 *
 *
 */

/* global settings */
#define SH_USE_WIFI
#define SH_USE_NTF

#define SH_USE_BT
#define SH_USE_LOGGER

//#define SH_LOGS_OVER_WIFI
#define SH_USE_RELAYS
//#define SH_USE_INPUTS
#define SH_USE_ENV
#define SH_USE_FAN/
//#define SH_USE_SLM
#define SH_USE_CMD_PARSER

void sm_setup_int_priorities()
{
   NVIC_SetPriorityGrouping(0x05);
   uint32_t prio;
   prio = NVIC_EncodePriority(0x05, -1, 0);
   NVIC_SetPriority(SysTick_IRQn, prio);

   /*
    * TIM4 is the timer used to controlling timeout on many synchronous function calls
    * therefore it has to have higher priority (to be able to interrupt systick interrupt).
    * Sometimes sync function are called from scheduler at high priority (called directly from scheduler interrupt)
    * and in such case TIM4 interrupt cannot be fired.
    */
   prio = NVIC_EncodePriority(0x05, -2, 0);
   NVIC_SetPriority(TIM4_IRQn, prio);
   prio = NVIC_EncodePriority(0x05, 2, 0);
   NVIC_SetPriority(USART1_IRQn, prio);

}


int main(void)
{
   sm_setup_int_priorities();

   ts_init();
	time_init();
	sch_initialize();

	/* BTengine is module shared between logger and command parser, therefore is always initialized */
	BT_Config config = {UART_COMMON_BAUD_RATE, UART_COMMON_BUFFER_SIZE, UART_COMMON_STRING_SIZE};
	btengine_initialize(&config);

   if (i2c_initialize() != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initialize I2C driver");
   }
   if (dht_initialize() != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initialize DHT driver");
   }

   /* set timeouts for I2C and DHT */
   i2c_set_timeout(I2C_TRANSACTION_TIMEOUT_MS);
   dht_set_timeout(DHT_TRANSACTION_TIMEOUT_MS);


#ifdef SH_USE_LOGGER
   if (logger_initialize(UART_COMMON_STRING_SIZE) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot init BT engine!");
   }
   logger_enable();
   logger_send(LOG_DEBUG, __func__, "Booting up!");
   logger_set_group_state(LOG_DEBUG, LOGGER_DEBUG_GROUP_STATE);
   logger_set_group_state(LOG_DHT_DRV, LOGGER_DHTDRV_GROUP_STATE);
   logger_set_group_state(LOG_WIFI_MANAGER, LOGGER_WIFIMGR_GROUP_STATE);
   logger_set_group_state(LOG_WIFI_DRIVER, LOGGER_WIFIDRV_GROUP_STATE);
   logger_set_group_state(LOG_I2C_DRV, LOGGER_I2CDRV_GROUP_STATE);
   logger_set_group_state(LOG_INPUTS, LOGGER_INPUTS_GROUP_STATE);
   logger_set_group_state(LOG_RELAYS, LOGGER_RELAYS_GROUP_STATE);
   logger_set_group_state(LOG_ENV, LOGGER_ENV_GROUP_STATE);
   logger_set_group_state(LOG_SLM, LOGGER_SLM_GROUP_STATE);
   logger_set_group_state(LOG_FAN, LOGGER_SLM_GROUP_STATE);
   if (logger_register_sender(&btengine_send_string) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot register BT sender!");
   }
#ifdef SH_LOGS_OVER_WIFI
   if (logger_register_sender(&wifimgr_broadcast_data) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot register WIFI sender!");
   }
#endif
#endif
   logger_send(LOG_ERROR, __func__, "Booting started!");


#ifdef SH_USE_WIFI
   WIFI_UART_Config wifi_cfg = {UART_COMMON_BAUD_RATE, UART_COMMON_BUFFER_SIZE, UART_COMMON_STRING_SIZE};
   if (wifimgr_initialize(&wifi_cfg) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initiailize wifi manager!");
   }
#endif


#ifdef SH_USE_CMD_PARSER
   cmd_register_sender(&btengine_send_string);

	if (btengine_register_callback(&cmd_handle_data) != RETURN_OK)
	{
		logger_send(LOG_ERROR, __func__, "Cannot add BT callback!");
	}
#endif


#ifdef SH_USE_INPUTS
	INPUTS_CONFIG inp_cfg;
	for (uint8_t i = 0; i < INPUTS_MAX_INPUT_LINES; i++)
	{
	   inp_cfg.items[i].item = INPUT_ENUM_COUNT;
	   inp_cfg.items[i].input_no = 0;
	}
	inp_cfg.address = INPUTS_I2C_ADDRESS;
   uint8_t inp_arr_idx = 0;
   for (uint8_t i = 1; i < (sizeof(INPUTS_MATCH_CONFIG)/sizeof(INPUTS_MATCH_CONFIG[0])); i++)
   {
      if (INPUTS_MATCH_CONFIG[i] != INPUT_ENUM_COUNT)
      {
         inp_cfg.items[inp_arr_idx].item = INPUTS_MATCH_CONFIG[i]; inp_cfg.items[inp_arr_idx].input_no = i;
         inp_arr_idx++;
      }
   }
   if (inp_initialize(&inp_cfg) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initialize Inputs board");
   }
#endif


#ifdef SH_USE_RELAYS
   RELAYS_CONFIG rel_cfg;
   for (uint8_t i = 0; i < RELAYS_BOARD_COUNT; i++)
   {
      rel_cfg.items[i].id = RELAY_ID_ENUM_MAX;
      rel_cfg.items[i].relay_no = 0;
   }
   rel_cfg.address = RELAYS_I2C_ADDRESS;
   uint8_t rel_arr_idx = 0;
   for (uint8_t i = 1; i < (sizeof(RELAYS_MATCH_CONFIG)/sizeof(RELAYS_MATCH_CONFIG[0])); i++)
   {
      if (RELAYS_MATCH_CONFIG[i] != RELAY_ID_ENUM_MAX)
      {
         rel_cfg.items[rel_arr_idx].id = RELAYS_MATCH_CONFIG[i]; rel_cfg.items[rel_arr_idx].relay_no = i;
         rel_arr_idx++;
      }
   }
   if (rel_initialize(&rel_cfg) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initialize Relays board");
   }
#endif


#ifdef SH_USE_ENV
   ENV_CONFIG env_cfg;
   env_cfg.measure_running = ENV_LOOP_MEASURE_RUNNING;
   env_cfg.max_cs_rate = ENV_MAX_CHECKSUM_RATE;
   env_cfg.max_nr_rate = ENV_MAX_NORESPONSE_RATE;
   for (uint8_t i = 0; i < (sizeof(ENV_MATCH_CONFIG)/sizeof(ENV_MATCH_CONFIG[0])); i++)
   {
      env_cfg.items[i].env_id = ENV_MATCH_CONFIG[i].env_id; env_cfg.items[i].dht_id = ENV_MATCH_CONFIG[i].dht_id;
   }
   if (env_initialize(&env_cfg) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initialize ENV module");
   }
#endif


#ifdef SH_USE_FAN
   FAN_CONFIG fan_cfg;
   fan_cfg.fan_humidity_threshold = FAN_HUMIDITY_THRESHOLD;
   fan_cfg.fan_threshold_hysteresis = FAN_THRESHOLD_HYSTERESIS;
   fan_cfg.max_working_time_s = FAN_MAX_WORKING_TIME_S;
   fan_cfg.min_working_time_s = FAN_MIN_WORKING_TIME_S;
   if (fan_initialize(&fan_cfg) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initialize FAN module");
   }
#endif


#ifdef SH_USE_SLM
   SLM_CONFIG slm_cfg;
   slm_cfg.address = SLM_I2C_ADDRESS;
   slm_cfg.off_effect_mode = SLM_OFF_EFFECT;
   slm_cfg.program_id = SLM_DEFAULT_PROGRAM_ID;
   if(slm_initialize(&slm_cfg) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initialize SLM module");
   }
#endif


#ifdef SH_USE_NTF
   ntfmgr_init();
#endif

   logger_send(LOG_ERROR, __func__, "Booting completed!");

	while (1)
	{
	   i2c_watcher();
	   dht_data_watcher();
		sch_task_watcher(TASKPRIO_LOW);
		btengine_string_watcher();
		uartengine_string_watcher();
	}
}
