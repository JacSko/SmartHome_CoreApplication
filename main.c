#include <stdlib.h>
#include "stm32f4xx.h"
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

#define UART_COMMON_BUFFER_SIZE 2048
#define UART_COMMON_STRING_SIZE 512
#define UART_COMMON_BAUD_RATE 115200
#define INPUTS_I2C_ADDRESS 0x48
#define RELAYS_I2C_ADDRESS 0x48
#define ENV_MEASUREMENT_RUNNING 0x00

void dummy_task()
{
   logger_send(LOG_ERROR, "", "S T R I N G S T R I N G S T R I N G S T R I N G S T R I N G");
}

int main(void)
{
   NVIC_SetPriorityGrouping(0x05);
   uint32_t prio;
   prio = NVIC_EncodePriority(0x05, 1, 0);
   NVIC_SetPriority(SysTick_IRQn, prio);

   /*
    * TIM4 is the timer used to controlling timeout on many synchronous function calls
    * therefore it has to have higher priority (to be able to interrupt systick interrupt).
    * Sometimes sync function are called from scheduler at high priority (called directly from scheduler interrupt)
    * and in such case TIM4 interrupt cannot be fired.
    */
   prio = NVIC_EncodePriority(0x05, -2, 0);
   NVIC_SetPriority(TIM4_IRQn, prio);

   ts_init();
	time_init();
	sch_initialize();
//	sch_subscribe_and_set(&dummy_task, TASKPRIO_LOW, 10, TASKSTATE_RUNNING, TASKTYPE_PERIODIC);

	BT_Config config = {UART_COMMON_BAUD_RATE, UART_COMMON_BUFFER_SIZE, UART_COMMON_STRING_SIZE};
	btengine_initialize(&config);

   if (logger_initialize(UART_COMMON_STRING_SIZE) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot init BT engine!");
   }
   logger_enable();

   logger_send(LOG_DEBUG, __func__, "Booting up!");

   logger_set_group_state(LOG_DHT_DRV, LOGGER_GROUP_ENABLE);
   logger_set_group_state(LOG_WIFI_MANAGER, LOGGER_GROUP_ENABLE);
   logger_set_group_state(LOG_WIFI_DRIVER, LOGGER_GROUP_ENABLE);
   logger_set_group_state(LOG_I2C_DRV, LOGGER_GROUP_ENABLE);
   logger_set_group_state(LOG_INPUTS, LOGGER_GROUP_ENABLE);
   logger_set_group_state(LOG_RELAYS, LOGGER_GROUP_ENABLE);
   logger_set_group_state(LOG_ENV, LOGGER_GROUP_ENABLE);
   if (logger_register_sender(&btengine_send_string) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot register BT sender!");
   }
//   if (logger_register_sender(&wifimgr_broadcast_data) != RETURN_OK)
//   {
//      logger_send(LOG_ERROR, __func__, "Cannot register WIFI sender!");
//   }

   WIFI_UART_Config wifi_cfg = {UART_COMMON_BAUD_RATE, UART_COMMON_BUFFER_SIZE, UART_COMMON_STRING_SIZE};
   if (wifimgr_initialize(&wifi_cfg) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initiailize wifi manager!");
   }

	cmd_register_sender(&btengine_send_string);

	if (btengine_register_callback(&cmd_handle_data) != RETURN_OK)
	{
		logger_send(LOG_ERROR, __func__, "Cannot add BT callback!");
	}

	if (i2c_initialize() != RETURN_OK)
	{
	   logger_send(LOG_ERROR, __func__, "Cannot initialize I2C driver");
	}
   if (dht_initialize() != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initialize DHT driver");
   }



	INPUTS_CONFIG inp_cfg;
	inp_cfg.address = INPUTS_I2C_ADDRESS;
	inp_cfg.items[0].item = INPUT_WARDROBE_LED; inp_cfg.items[0].input_no = 1;
	inp_cfg.items[1].item = INPUT_BATHROOM_LED; inp_cfg.items[1].input_no = 2;
	inp_cfg.items[2].item = INPUT_SOCKETS; inp_cfg.items[2].input_no = 9;
	inp_cfg.items[3].item = INPUT_BEDROOM_AC; inp_cfg.items[3].input_no = 10;
   inp_cfg.items[4].item = INPUT_WARDROBE_AC; inp_cfg.items[4].input_no = 11;
   inp_cfg.items[5].item = INPUT_KITCHEN_AC; inp_cfg.items[5].input_no = 12;
   inp_cfg.items[6].item = INPUT_BATHROOM_AC; inp_cfg.items[6].input_no = 13;
   inp_cfg.items[7].item = INPUT_STAIRS_AC; inp_cfg.items[7].input_no = 14;
   inp_cfg.items[8].item = INPUT_STAIRS_SENSOR; inp_cfg.items[8].input_no = 15;
   inp_cfg.items[9].item = INPUT_KITCHEN_WALL; inp_cfg.items[9].input_no = 16;
   if (inp_initialize(&inp_cfg) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initialize Inputs board");
   }

   RELAYS_CONFIG rel_cfg;
   rel_cfg.address = RELAYS_I2C_ADDRESS;
   rel_cfg.items[0].id = RELAY_WARDROBE_LED; rel_cfg.items[0].relay_no = 1;
   rel_cfg.items[1].id = RELAY_BATHROOM_LED; rel_cfg.items[1].relay_no = 2;
   rel_cfg.items[2].id = RELAY_STAIRCASE_LED; rel_cfg.items[2].relay_no = 8;
   rel_cfg.items[3].id = RELAY_SOCKETS; rel_cfg.items[3].relay_no = 9;
   rel_cfg.items[4].id = RELAY_BATHROOM_FAN; rel_cfg.items[4].relay_no = 10;
   rel_cfg.items[5].id = RELAY_KITCHEN_WALL; rel_cfg.items[5].relay_no = 11;
   rel_cfg.items[6].id = RELAY_STAIRCASE_AC; rel_cfg.items[6].relay_no = 12;
   rel_cfg.items[7].id = RELAY_BATHROOM_AC; rel_cfg.items[7].relay_no = 13;
   rel_cfg.items[8].id = RELAY_KITCHEN_AC; rel_cfg.items[8].relay_no = 14;
   rel_cfg.items[9].id = RELAY_BEDROOM_AC; rel_cfg.items[9].relay_no = 15;
   rel_cfg.items[10].id = RELAY_WARDROBE_AC; rel_cfg.items[10].relay_no = 16;
   if (rel_initialize(&rel_cfg) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initialize Relays board");
   }

   ENV_CONFIG env_cfg;
   env_cfg.measure_running = ENV_MEASUREMENT_RUNNING;
   env_cfg.max_cs_rate = 90;
   env_cfg.max_nr_rate = 90;
   env_cfg.items[0].env_id = ENV_OUTSIDE; env_cfg.items[0].dht_id = DHT_SENSOR1;
   env_cfg.items[1].env_id = ENV_WARDROBE; env_cfg.items[1].dht_id = DHT_SENSOR2;
   env_cfg.items[2].env_id = ENV_BEDROOM; env_cfg.items[2].dht_id = DHT_SENSOR3;
   env_cfg.items[3].env_id = ENV_BATHROOM; env_cfg.items[3].dht_id = DHT_SENSOR4;
   env_cfg.items[4].env_id = ENV_KITCHEN; env_cfg.items[4].dht_id = DHT_SENSOR5;
   env_cfg.items[5].env_id = ENV_STAIRS; env_cfg.items[5].dht_id = DHT_SENSOR6;
   if (env_initialize(&env_cfg) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initialize ENV module");
   }

   FAN_CONFIG fan_cfg;
   fan_cfg.fan_humidity_threshold = 70;
   fan_cfg.fan_threshold_hysteresis = 5;
   fan_cfg.max_working_time_s = 7200;
   fan_cfg.min_working_time_s = 600;
   if (fan_initialize(&fan_cfg) != RETURN_OK)
   {
      logger_send(LOG_ERROR, __func__, "Cannot initialize FAN module");
   }


   logger_send(LOG_ERROR, __func__, "Booting completed!");

	while (1)
	{
	   i2c_watcher();
		sch_task_watcher(TASKPRIO_LOW);
		btengine_string_watcher();
		uartengine_string_watcher();
	}
}
