
#include "stm32f4xx.h"
#include "uart_engine.h"

void callback(const char * data)
{
	uartengine_send_string(data);
}

int main(void)
{
	UART_Config config = {115200, '\n', 1024, 512};
	uartengine_initialize(&config);
	uartengine_register_callback(&callback);
	/*simple uart echo*/
	while (1)
	{
		uartengine_string_watcher();
	}
}
