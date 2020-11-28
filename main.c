
#include "stm32f4xx.h"
#include "time_counter.h"
#include "wifi_driver.h"
#include "uart_engine.h"
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
	if (wifi_initialize() != RETURN_OK)
	{
		while(1){};
	}
	RET_CODE result = RETURN_OK;

	if (wifi_connect_to_network("NIE_KRADNIJ_INTERNETU!!!", "radionet0098") == RETURN_OK)
	{
		if (wifi_allow_multiple_clients(1) == RETURN_OK)
		{
			if (wifi_open_udp_server(2222))
			{

			}
		}
	}



	while (1)
	{
	}
}
