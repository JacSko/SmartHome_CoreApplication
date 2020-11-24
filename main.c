
#include "stm32f4xx.h"
#include "time_counter.h"
#include "wifi_driver.h"
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
	wifi_initialize();

	wifi_connect_to_network("NIE_KRADNIJ_INTERNETU!!!", "radionet0098");

	while (1)
	{
	}
}
