/*
 * uart_receiver.c
 *
 *  Created on: Jul 17, 2024
 *      Author: Darisz Strojny @ReijiY
 */

#include "uart_receiver.h"

uint8_t UartRcvr_init(uart_receiver_t* uart_rcvr, UART_HandleTypeDef* huart)
{
	if(uart_rcvr == NULL || huart == NULL) return 0;
	UartRcvr_deinit(uart_rcvr);

	uart_rcvr->buffer_active.data = (uint8_t*)malloc(UART_RECEIVER_CIRCULAR_BUFFER_SIZE);
	uart_rcvr->buffer_active.len = 0;
	if (uart_rcvr->buffer_active.data == NULL)
	{
		UartRcvr_deinit(uart_rcvr);
		return 0;
	}

	uart_rcvr->buffer_pending.data = (uint8_t*)malloc(UART_RECEIVER_MAX_BUFFER_LENGTH);
	uart_rcvr->buffer_pending.len = 0;
	if (uart_rcvr->buffer_pending.data == NULL)
	{
		UartRcvr_deinit(uart_rcvr);
		return 0;
	}

	uart_rcvr->huart = huart;

	HAL_UARTEx_ReceiveToIdle_DMA(huart, uart_rcvr->buffer_active.data, UART_RECEIVER_CIRCULAR_BUFFER_SIZE);
	return 1;
}

void UartRcvr_deinit(uart_receiver_t *uart_rcvr)
{
	if(uart_rcvr == NULL) return;
	if(uart_rcvr->buffer_active.data != NULL) free(uart_rcvr->buffer_active.data);
	if(uart_rcvr->buffer_pending.data != NULL) free(uart_rcvr->buffer_pending.data);
	for (uint8_t i = 0; i < UART_RECEIVER_MAX_BUFFERS; i++)
	{
		if(uart_rcvr->buffers[i].data != NULL) free(uart_rcvr->buffers[i].data);
	}
	memset(uart_rcvr, 0, sizeof(uart_receiver_t));
}

uint8_t UartRcvr_available(uart_receiver_t *uart_rcvr)
{
	if (uart_rcvr == NULL || uart_rcvr->huart == NULL) return 0;

	while (uart_rcvr->buffer_index_pending != uart_rcvr->buffer_index_active
			&& uart_rcvr->buffers[uart_rcvr->buffer_index_pending].data == NULL)
	{
		uart_rcvr->buffer_index_pending = (uart_rcvr->buffer_index_pending + 1) % UART_RECEIVER_MAX_BUFFERS;
	}

	return uart_rcvr->buffers[uart_rcvr->buffer_index_pending].data != NULL;
}

uint16_t UartRcvr_get_input(uart_receiver_t *uart_rcvr, uint8_t* str, uint16_t max_len)
{
	if (uart_rcvr == NULL || uart_rcvr->huart == NULL) return 0;

	if (uart_rcvr->buffers[uart_rcvr->buffer_index_pending].data == NULL) return 0;

	uint16_t len = uart_rcvr->buffers[uart_rcvr->buffer_index_pending].len;
	if(str != NULL && max_len > 0)
	{
		if (len + 1 > max_len) len = max_len - 1;
		memcpy(str, uart_rcvr->buffers[uart_rcvr->buffer_index_pending].data + uart_rcvr->buffers[uart_rcvr->buffer_index_pending].processed, len + 1);
		str[len] = '\0';
		if (len != uart_rcvr->buffers[uart_rcvr->buffer_index_pending].len)
		{
			uart_rcvr->buffers[uart_rcvr->buffer_index_pending].len -= len;
			uart_rcvr->buffers[uart_rcvr->buffer_index_pending].processed += len;
		}
	}

	if(len == uart_rcvr->buffers[uart_rcvr->buffer_index_pending].len || str == NULL)
	{
		free(uart_rcvr->buffers[uart_rcvr->buffer_index_pending].data);
		uart_rcvr->buffers[uart_rcvr->buffer_index_pending].data = NULL;
	}
	return len;
}

uint8_t UartRcvr_set_baud_rate(uart_receiver_t *uart_rcvr, uint32_t baud_rate)
{
	if(uart_rcvr == NULL || uart_rcvr->huart == NULL) return 0;

	uint32_t pclk;
	if (uart_rcvr->huart->Instance == USART1 || uart_rcvr->huart->Instance == USART6) {
		// USART1 and USART6 are on APB2
		pclk = HAL_RCC_GetPCLK2Freq();
	} else {
		// USART2, USART3, UART4, UART5 are on APB1
		pclk = HAL_RCC_GetPCLK1Freq();
	}
    uint32_t usartdiv = (pclk + (baud_rate / 2)) / baud_rate;

    UART_HandleTypeDef* huart = uart_rcvr->huart;
    USART_TypeDef* instance = huart->Instance;


    while(__HAL_UART_GET_FLAG(huart, UART_FLAG_TXE) == RESET); // wait for transmission complete

    instance->CR1 &= ~(USART_CR1_UE);
	__DSB(); // barrier
	(void)(instance->CR1); // read-back
	__DMB();

    instance->BRR = usartdiv;
    __DSB();
    (void)(instance->BRR);
    __DMB();

    instance->CR1 |= USART_CR1_UE;
	__DSB();
	(void)(instance->CR1);
	__DMB();

	return 1;
}

HAL_StatusTypeDef UartRcvr_send_char(uart_receiver_t *uart_rcvr, char c)
{
	if(uart_rcvr == NULL || uart_rcvr->huart == NULL) return HAL_ERROR;
	return HAL_UART_Transmit(uart_rcvr->huart, (uint8_t*)&c, 1, UART_TX_TIMEOUT);
}

HAL_StatusTypeDef UartRcvr_send(uart_receiver_t *uart_rcvr, const uint8_t* str, uint16_t len)
{
	if(uart_rcvr == NULL || uart_rcvr->huart == NULL) return HAL_ERROR;
	return HAL_UART_Transmit(uart_rcvr->huart, str, len, UART_TX_TIMEOUT);
}

HAL_StatusTypeDef UartRcvr_print(uart_receiver_t *uart_rcvr, const char* str)
{
	return UartRcvr_send(uart_rcvr, (const uint8_t*)str, strlen(str));
}

void UartRcvr_it_swap(uart_receiver_t *uart_rcvr)
{
	if (uart_rcvr == NULL || uart_rcvr->huart == NULL) return;

	uint16_t next_index = (uart_rcvr->buffer_index_active + 1) % UART_RECEIVER_MAX_BUFFERS;
	if (uart_rcvr->buffers[next_index].data != NULL) return;

	uart_rcvr->buffers[next_index].data = (uint8_t*)malloc(uart_rcvr->buffer_pending.len + 1);
	if (uart_rcvr->buffers[next_index].data == NULL) return;

	memcpy(uart_rcvr->buffers[next_index].data, uart_rcvr->buffer_pending.data, uart_rcvr->buffer_pending.len);
	uart_rcvr->buffers[next_index].data[uart_rcvr->buffer_pending.len] = '\0';
	uart_rcvr->buffers[next_index].len = uart_rcvr->buffer_pending.len;
	uart_rcvr->buffers[next_index].processed = 0;

	uart_rcvr->buffer_index_active = next_index;
	uart_rcvr->buffer_pending.len = 0;
}


void UartRcvr_it_process(uart_receiver_t *uart_rcvr, uint16_t offset, uint16_t size)
{
	uint16_t size_left = ((uart_rcvr->buffer_pending.len + size) <= UART_RECEIVER_MAX_BUFFER_LENGTH)
			? size
			: (UART_RECEIVER_MAX_BUFFER_LENGTH - uart_rcvr->buffer_pending.len);
	if(size_left != 0)
	{
		memcpy(uart_rcvr->buffer_pending.data + uart_rcvr->buffer_pending.len, uart_rcvr->buffer_active.data + offset, size_left);
		uart_rcvr->buffer_pending.len += size_left;
	}
	if (size_left != size) {
		UartRcvr_it_swap(uart_rcvr);
		UartRcvr_it_process(uart_rcvr, (offset + size_left) % UART_RECEIVER_CIRCULAR_BUFFER_SIZE, size - size_left);
	}
}

void UartRcvr_it_trigger(uart_receiver_t *uart_rcvr, uint16_t pos)
{
	if(uart_rcvr == NULL || uart_rcvr->huart == NULL) return;
	if(uart_rcvr->old_pos != pos)
	{
		if(pos > uart_rcvr->old_pos)
		{
			UartRcvr_it_process(uart_rcvr, uart_rcvr->old_pos, pos - uart_rcvr->old_pos);
		}
		else
		{
			UartRcvr_it_process(uart_rcvr, uart_rcvr->old_pos, UART_RECEIVER_CIRCULAR_BUFFER_SIZE - uart_rcvr->old_pos);
			if(pos > 0)
			{
				UartRcvr_it_process(uart_rcvr, 0, pos);
			}
		}
		uart_rcvr->old_pos = pos;
	}
}

void UartRcvr_it_error(uart_receiver_t *uart_rcvr)
{
	if (uart_rcvr == NULL || uart_rcvr->huart == NULL) return;
	uart_rcvr->old_pos = 0;
	HAL_UARTEx_ReceiveToIdle_DMA(uart_rcvr->huart, uart_rcvr->buffer_active.data, UART_RECEIVER_CIRCULAR_BUFFER_SIZE);
}
