#include <stdlib.h>
#include <string.h>

#include "stm32f4xx.h"
#include "core_cmFunc.h"
#include "bt_engine.h"
#include "gpio_lib.h"
#include "return_codes.h"
/*
 * UART engine summary:
 * - UART1 peripherial
 * - UART_TX = PA9
 * - UART_RX = PA10
 */

/* INTERNAL functions declaration */
void btengine_start_sending();
void btengine_stop_sending();
void btengine_notify_callbacks();
#define BT_ENGINE_CALLBACK_SIZE 5
RET_CODE btengine_get_string_from_buffer();

typedef struct
{
	char * buf;
	uint16_t tail;
	uint16_t head;
	uint8_t string_cnt;
	uint16_t bytes_cnt;
} BT_BUFFER;

BT_Config config;
volatile BT_BUFFER bt_tx_buf;
volatile BT_BUFFER bt_rx_buf;
char* rx_string;

void (*BT_CALLBACKS[BT_ENGINE_CALLBACK_SIZE])(const char *);


RET_CODE btengine_initialize(BT_Config* cfg)
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

	bt_tx_buf.buf = (char*) malloc (sizeof(char)*config.buffer_size);
	bt_rx_buf.buf = (char*) malloc (sizeof(char)*config.buffer_size);
	rx_string = (char*) malloc (sizeof(char)*config.string_size);

	bt_tx_buf.head = 0;
	bt_tx_buf.tail = 0;
	bt_tx_buf.string_cnt = 0; /* not used */
	bt_tx_buf.bytes_cnt = 0;  /* not user */

	bt_rx_buf.head = 0;
	bt_rx_buf.tail = 0;
	bt_rx_buf.string_cnt = 0;
	bt_rx_buf.bytes_cnt = 0;

	if (!bt_rx_buf.buf || !bt_rx_buf.buf || !rx_string)
	{
		result = RETURN_ERROR;
	}

	NVIC_EnableIRQ(USART1_IRQn);

	return result;
}

void btengine_deinitialize()
{
	free(bt_tx_buf.buf);
	free(bt_rx_buf.buf);
	free(rx_string);
	bt_tx_buf.buf = NULL;
	bt_rx_buf.buf = NULL;
	rx_string = NULL;
	for (uint8_t i = 0; i < BT_ENGINE_CALLBACK_SIZE; i++)
	{
		BT_CALLBACKS[i] = NULL;
	}

}

RET_CODE btengine_send_string(const char * buffer)
{
	if (!bt_tx_buf.buf)
	{
		return RETURN_ERROR;
	}

	char c;
	__disable_irq();
	while (*buffer)
	{
		c = *buffer;
		bt_tx_buf.buf[bt_tx_buf.head] = c;
		bt_tx_buf.head++;
		if (bt_tx_buf.head == config.buffer_size)
		{
			bt_tx_buf.head = 0;
		}
		buffer++;
	}
	__enable_irq();
	btengine_start_sending();
	return RETURN_OK;
}

RET_CODE btengine_can_read_string()
{
	RET_CODE result = RETURN_NOK;

	if (bt_rx_buf.string_cnt > 0)
	{
		btengine_get_string_from_buffer();
		if (strlen(rx_string) > 0)
		{
			result = RETURN_OK;
		}
	}
	return result;
}

const char* btengine_get_string()
{
	return rx_string;
}

void btengine_clear_rx()
{
	bt_rx_buf.tail = bt_rx_buf.head;
	bt_rx_buf.string_cnt = 0;
	bt_rx_buf.bytes_cnt = 0;
}

RET_CODE btengine_get_string_from_buffer()
{
	char* buffer = rx_string;
	if (!bt_rx_buf.buf || bt_rx_buf.string_cnt == 0 || !buffer)
	{
		return RETURN_ERROR;
	}

	char c;
	while (1)
	{
		c = bt_rx_buf.buf[bt_rx_buf.tail];
		bt_rx_buf.tail++;
		bt_rx_buf.bytes_cnt--;
		if (bt_rx_buf.tail == config.buffer_size)
		{
			bt_rx_buf.tail = 0;
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
	bt_rx_buf.string_cnt--;
	return RETURN_OK;
}

uint16_t btengine_count_bytes()
{
	return bt_rx_buf.bytes_cnt;
}

const uint8_t* btengine_get_bytes()
{
	char* buffer = rx_string;
	if (!bt_rx_buf.buf || bt_rx_buf.bytes_cnt == 0 || !buffer)
	{
		return NULL;
	}

	while (bt_rx_buf.bytes_cnt)
	{
		*buffer = bt_rx_buf.buf[bt_rx_buf.tail];
		bt_rx_buf.tail++;
		bt_rx_buf.bytes_cnt--;
		if (bt_rx_buf.tail == config.buffer_size)
		{
			bt_rx_buf.tail = 0;
		}
		buffer++;

	}
	return (uint8_t*)rx_string;
}

RET_CODE btengine_register_callback(void(*callback)(const char *))
{
	RET_CODE result = RETURN_ERROR;
	for (uint8_t i = 0; i < BT_ENGINE_CALLBACK_SIZE; i++)
	{
		if (BT_CALLBACKS[i] == NULL)
		{
			BT_CALLBACKS[i] = callback;
			result = RETURN_OK;
			break;
		}
	}
	return result;
}

RET_CODE btengine_unregister_callback(void(*callback)(const char *))
{
	RET_CODE result = RETURN_ERROR;
	for (uint8_t i = 0; i < BT_ENGINE_CALLBACK_SIZE; i++)
	{
		if (BT_CALLBACKS[i] == callback)
		{
			BT_CALLBACKS[i] = NULL;
			result = RETURN_OK;
			break;
		}
	}
	return result;
}

void btengine_notify_callbacks()
{
	for (uint8_t i = 0; i < BT_ENGINE_CALLBACK_SIZE; i++)
	{
		if (BT_CALLBACKS[i] != NULL)
		{
			BT_CALLBACKS[i](rx_string);
		}
	}
}

void btengine_string_watcher()
{
	if (btengine_can_read_string())
	{
		btengine_notify_callbacks();
	}
}

void btengine_start_sending()
{
	USART1->CR1 |= USART_CR1_TXEIE;
}
void btengine_stop_sending()
{
	USART1->CR1 &= ~USART_CR1_TXEIE;
}

void USART1_IRQHandler (void)
{
	if (USART1->SR & USART_SR_RXNE){
		char c = USART1->DR;
		bt_rx_buf.buf[bt_rx_buf.head] = USART1->DR;
		bt_rx_buf.head++;
		bt_rx_buf.bytes_cnt++;
		if (c == '\n')
		{
			bt_rx_buf.string_cnt++;
		}

		if (bt_rx_buf.head == config.buffer_size)
		{
			bt_rx_buf.head = 0;
		}

	}

	if (USART1->SR & USART_SR_TXE){
		if (bt_tx_buf.head == bt_tx_buf.tail)
		{
			btengine_stop_sending();
		}
		else
		{
			USART1->DR = bt_tx_buf.buf[bt_tx_buf.tail];
			bt_tx_buf.tail++;
			if (bt_tx_buf.tail == config.buffer_size)
			{
				bt_tx_buf.tail = 0;
			}
		}
	}
}
