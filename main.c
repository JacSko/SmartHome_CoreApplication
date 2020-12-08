
#include "stm32f4xx.h"
#include "time_counter.h"
#include "wifi_manager.h"
#include "uart_engine.h"
#include "bt_engine.h"
#include "task_scheduler.h"
#include "Logger.h"
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

void print_log()
{
	TimeItem item = {};
	RET_CODE result = wifimgr_get_time(&item);
	logger_send(LOG_DEBUG, __func__,"Get time result %d: %d:%d%d %d-%d-%d", result,
							item.day, item.month, item.year,
							item.hour, item.minute, item.second);
}

int main(void)
{
	time_init();
	sch_initialize();
	sch_subscribe(&print_log);
	sch_set_task_period(&print_log, 20000);
	sch_set_task_type(&print_log, TASKTYPE_PERIODIC);
	sch_set_task_state(&print_log, TASKSTATE_RUNNING);

	BT_Config config = {115200, 2048, 1024};
	btengine_initialize(&config);
	logger_initialize(1024);
	logger_register_sender(&btengine_send_string);
	logger_enable();
	logger_set_group_state(LOG_DEBUG, LOGGER_GROUP_ENABLE);
	logger_set_group_state(LOG_WIFI_DRIVER, LOGGER_GROUP_ENABLE);
	logger_set_group_state(LOG_WIFI_MANAGER, LOGGER_GROUP_ENABLE);
	logger_send(LOG_DEBUG, __FILE__, "Booting up!");

	if (wifimgr_initialize() == RETURN_OK)
	{
		logger_register_sender(&wifimgr_broadcast_data);
		logger_send(LOG_DEBUG, __func__, "Wifi manager started");
	}


	logger_send(LOG_DEBUG, __FILE__, "Booting completed!");
	while (1)
	{
		sch_task_watcher();
		btengine_string_watcher();
		uartengine_string_watcher();
	}
}
