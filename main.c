
#include "stm32f4xx.h"
#include "uart_engine.h"
#include "time_counter.h"

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

void callback(const char * data)
{
	uartengine_send_string(data);
}

int main(void)
{
	UART_Config config = {115200, '\n', 1024, 512};
	uartengine_initialize(&config);
	uartengine_register_callback(&callback);

	time_init();

	/*simple uart echo*/
	while (1)
	{
		uartengine_string_watcher();
		time_watcher();
	}
}
