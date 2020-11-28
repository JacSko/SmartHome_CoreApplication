#include <stdlib.h>
#include <string.h>

#include "stm32f4xx.h"
#include "core_cmFunc.h"
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
void uartengine_notify_callbacks();
#define UART_ENGINE_CALLBACK_SIZE 5
RET_CODE uartengine_get_string_from_buffer();

typedef struct
{
	char * buf;
	uint16_t tail;
	uint16_t head;
	uint8_t string_cnt;
	uint16_t bytes_cnt;
}BUFFER;

UART_Config config;
volatile BUFFER tx_buf;
volatile BUFFER rx_buf;
char* rx_string;

void (*CALLBACKS[UART_ENGINE_CALLBACK_SIZE])(const char *);


RET_CODE uartengine_initialize(UART_Config* cfg)
{
	RET_CODE result = RETURN_OK;
	config = *cfg;

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	__DSB();

	USART1->CR1 |= USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_TE;
	USART1->BRR = (100000000/config.baudrate);
	gpio_pin_cfg(GPIOA, PA10, gpio_mode_AF7_OD_PU_LS);
	gpio_pin_cfg(GPIOA, PA9, gpio_mode_AF7_PP_LS);

	tx_buf.buf = (char*) malloc (sizeof(char)*config.buffer_size);
	rx_buf.buf = (char*) malloc (sizeof(char)*config.buffer_size);
	rx_string = (char*) malloc (sizeof(char)*config.string_size);

	tx_buf.head = 0;
	tx_buf.tail = 0;
	tx_buf.string_cnt = 0; /* not used */
	tx_buf.bytes_cnt = 0;  /* not user */

	rx_buf.head = 0;
	rx_buf.tail = 0;
	rx_buf.string_cnt = 0;
	rx_buf.bytes_cnt = 0;

	if (!rx_buf.buf || !rx_buf.buf || !rx_string)
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
	free(rx_string);
	tx_buf.buf = NULL;
	rx_buf.buf = NULL;
	rx_string = NULL;
	for (uint8_t i = 0; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		CALLBACKS[i] = NULL;
	}

}

RET_CODE uartengine_send_string(const char * buffer)
{
	if (!tx_buf.buf)
	{
		return RETURN_ERROR;
	}

	char c;
	__disable_irq();
	while (*buffer)
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
	__enable_irq();
	start_sending();
	return RETURN_OK;
}

RET_CODE uartengine_can_read_string()
{
	RET_CODE result = RETURN_NOK;

	if (rx_buf.string_cnt > 0)
	{
		uartengine_get_string_from_buffer();
		if (strlen(rx_string) > 0)
		{
			result = RETURN_OK;
		}
	}
	return result;
}

const char* uartengine_get_string()
{
	return rx_string;
}

RET_CODE uartengine_get_string_from_buffer()
{
	char* buffer = rx_string;
	if (!rx_buf.buf || rx_buf.string_cnt == 0 || !buffer)
	{
		return RETURN_ERROR;
	}

	char c;
	while (1)
	{
		c = rx_buf.buf[rx_buf.tail];
		rx_buf.tail++;
		rx_buf.bytes_cnt--;
		if (rx_buf.tail == config.buffer_size)
		{
			rx_buf.tail = 0;
		}
		if (c != '\r' && c != '\n')
		{
			*buffer = c;
		}
		else
		{
			*buffer = 0x00;
		}
		if (c == '\n')
		{
			*buffer = 0x00;
			break;
		}
		buffer++;

	}
	rx_buf.string_cnt--;
	return RETURN_OK;
}

uint16_t uartengine_count_bytes()
{
	return rx_buf.bytes_cnt;
}

const uint8_t* uartengine_get_bytes()
{
	char* buffer = rx_string;
	if (!rx_buf.buf || rx_buf.bytes_cnt == 0 || !buffer)
	{
		return NULL;
	}

	while (rx_buf.bytes_cnt)
	{
		*buffer = rx_buf.buf[rx_buf.tail];
		rx_buf.tail++;
		rx_buf.bytes_cnt--;
		if (rx_buf.tail == config.buffer_size)
		{
			rx_buf.tail = 0;
		}
		buffer++;

	}
	return (uint8_t*)rx_string;
}

RET_CODE uartengine_register_callback(void(*callback)(const char *))
{
	RET_CODE result = RETURN_ERROR;
	for (uint8_t i = 0; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		if (CALLBACKS[i] == NULL)
		{
			CALLBACKS[i] = callback;
			result = RETURN_OK;
			break;
		}
	}
	return result;
}

RET_CODE uartengine_unregister_callback(void(*callback)(const char *))
{
	RET_CODE result = RETURN_ERROR;
	for (uint8_t i = 0; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		if (CALLBACKS[i] == callback)
		{
			CALLBACKS[i] = NULL;
			result = RETURN_OK;
			break;
		}
	}
	return result;
}

void uartengine_notify_callbacks()
{
	for (uint8_t i = 0; i < UART_ENGINE_CALLBACK_SIZE; i++)
	{
		if (CALLBACKS[i] != NULL)
		{
			CALLBACKS[i](rx_string);
		}
	}
}

void uartengine_string_watcher()
{
	if (uartengine_can_read_string())
	{
		uartengine_notify_callbacks();
	}
}

void start_sending()
{
	USART1->CR1 |= USART_CR1_TXEIE;
}
void stop_sending()
{
	USART1->CR1 &= ~USART_CR1_TXEIE;
}

void USART1_IRQHandler (void)
{
	if (USART1->SR & USART_SR_RXNE){
		char c = USART1->DR;
		rx_buf.buf[rx_buf.head] = USART1->DR;
		rx_buf.head++;
		rx_buf.bytes_cnt++;
		if (c == '\n')
		{
			rx_buf.string_cnt++;
		}

		if (rx_buf.head == config.buffer_size)
		{
			rx_buf.head = 0;
		}

	}

	if (USART1->SR & USART_SR_TXE){
		if (tx_buf.head == tx_buf.tail)
		{
			stop_sending();
		}
		else
		{
			USART1->DR = tx_buf.buf[tx_buf.tail];
			tx_buf.tail++;
			if (tx_buf.tail == config.buffer_size)
			{
				tx_buf.tail = 0;
			}
		}
	}
}
