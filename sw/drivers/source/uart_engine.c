#include <stdlib.h>

#include "stm32f4xx.h"
#include "uart_engine.h"
#include "gpio_lib.h"
#include "return_codes.h"
/*
 * UART engine summary:
 * - UART1 peripherial
 * - UART_TX = PA9
 * - UART_RX = PA10
 */

/* INTERNAL functions declaration */
void start_sending();
void stop_sending();



typedef struct
{
	char * buf;
	uint16_t tail;
	uint16_t head;
	uint8_t string_cnt;
}BUFFER;

UART_Config config;
BUFFER tx_buf;
BUFFER rx_buf;

RET_CODE uartengine_initialize(UART_Config* cfg)
{
	RET_CODE result = RETURN_OK;
	config = *cfg;

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	__DSB();

	USART1->CR1 |= USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_TE;
	USART1->BRR = (50000000/config.baudrate);
	gpio_pin_cfg(GPIOA, PA10, gpio_mode_AF7_OD_PD_LS);
	gpio_pin_cfg(GPIOA, PA9, gpio_mode_AF7_PP_LS);

	tx_buf.buf = (char*) malloc(sizeof(char)*config.buffer_size);
	rx_buf.buf = (char*) malloc(sizeof(char)*config.buffer_size);

	tx_buf.head = 0;
	tx_buf.tail = 0;
	tx_buf.string_cnt = 0;

	rx_buf.head = 0;
	rx_buf.tail = 0;
	rx_buf.string_cnt = 0;

	if (!rx_buf.buf || !rx_buf.buf)
	{
		result = RETURN_ERROR;
	}

	NVIC_EnableIRQ(USART1_IRQn);

	return result;
}

void uartengine_deinitialize()
{
	free(tx_buf.buf);
	free(rx_buf.buf);
}

RET_CODE uartengine_send_string(const char * buffer)
{
	if (!tx_buf.buf)
	{
		return RETURN_ERROR;
	}
	char c;
	while (buffer)
	{
		c = *buffer;
		tx_buf.buf[tx_buf.head] = c;
		tx_buf.head++;
		if (tx_buf.head == config.buffer_size)
		{
			tx_buf.head = 0;
		}
		buffer++;
	}

	return RETURN_OK;

}
const char* uartengine_get_string()
{

}

RET_CODE uartengine_is_string()
{
	return (RET_CODE) (rx_buf.string_cnt > 0);
}

void start_sending()
{

}
void stop_sending()
{

}

void USART1_IRQHandler (void)
{
//	if (USART1->SR & USART_SR_RXNE){
//	}
//
//	if (USART1->SR & USART_SR_TXE){
//	}
}
