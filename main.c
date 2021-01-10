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
 *  APB1 prescaler - 2 - 50MHz
 *  APB2 prescaler - 1 - 100MHz
 *
 *
 */

#define UART_COMMON_BUFFER_SIZE 1024
#define UART_COMMON_STRING_SIZE 512
#define UART_COMMON_BAUD_RATE 115200

void print_log()
{
   logger_send(LOG_ERROR,  __func__,"test");
   sch_trigger_task(&print_log);
}

int main(void)
{
	time_init();
	sch_initialize();
	sch_subscribe(&print_log);
	sch_set_task_period(&print_log, 20000);
	sch_set_task_type(&print_log, TASKTYPE_TRIGGER);

	BT_Config config = {UART_COMMON_BAUD_RATE, UART_COMMON_BUFFER_SIZE, UART_COMMON_STRING_SIZE};
	btengine_initialize(&config);
	logger_initialize(UART_COMMON_STRING_SIZE);
	logger_register_sender(&btengine_send_string);
	logger_enable();
	logger_set_group_state(LOG_DEBUG, LOGGER_GROUP_ENABLE);
	logger_set_group_state(LOG_WIFI_DRIVER, LOGGER_GROUP_ENABLE);
	logger_set_group_state(LOG_WIFI_MANAGER, LOGGER_GROUP_ENABLE);
	logger_set_group_state(LOG_INPUTS, LOGGER_GROUP_ENABLE);
	logger_set_group_state(LOG_RELAYS, LOGGER_GROUP_ENABLE);
	logger_set_group_state(LOG_SLM, LOGGER_GROUP_ENABLE);
	logger_send(LOG_DEBUG, __FILE__, "Booting up!");

	cmd_register_bt_sender(&btengine_send_string);
	if (btengine_register_callback(&cmd_handle_bt_data) != RETURN_OK)
	{
		logger_send(LOG_ERROR, __FILE__, "Cannot add BT callback!");
	}

	logger_send(LOG_DEBUG, __FILE__, "Booting completed!");
	i2c_initialize();
	sch_trigger_task(&print_log);

	SLM_CONFIG led_config;
	led_config.address = 0x48;
	led_config.off_effect_mode = SLM_OFF_EFFECT_ENABLED;
	led_config.program_id = SLM_PROGRAM1;

	slm_initialize(&led_config);


	while (1)
	{
	   i2c_watcher();
		sch_task_watcher(TASKPRIO_LOW);
		btengine_string_watcher();
		uartengine_string_watcher();
	}
}
