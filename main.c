
#include "stm32f4xx.h"
#include "time_counter.h"
#include "wifi_manager.h"
#include "uart_engine.h"
#include "bt_engine.h"
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

int main(void)
{
	time_init();
	BT_Config config = {115200, 1024, 512};
	btengine_initialize(&config);
	logger_initialize(512, &btengine_send_string);
	logger_enable();
	logger_set_group_state(LOG_DEBUG, LOGGER_GROUP_ENABLE);
	logger_set_group_state(LOG_WIFI_DRIVER, LOGGER_GROUP_ENABLE);
	logger_send(LOG_DEBUG, __FILE__, "Booting up!");

	wifimgr_initialize();

	while (1)
	{
	}
}
