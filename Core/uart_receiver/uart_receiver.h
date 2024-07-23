/*
 * uart_receiver.h
 *
 *  Created on: Jul 17, 2024
 *      Author: Darisz Strojny @ReijiY
 */


#ifndef UART_RECEIVER__H_
#define UART_RECEIVER__H_

#define UART_RECEIVER_CIRCULAR_BUFFER_SIZE (512 * sizeof(uint8_t))
// max size + null terminator
#define UART_RECEIVER_MAX_BUFFER_LENGTH (256 * sizeof(uint8_t))
#define UART_RECEIVER_MAX_BUFFERS 10


#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "stm32f4xx_hal.h"

typedef struct {
	uint8_t* data;
	uint16_t len;
} uart_receiver_buffer_t;

typedef struct {
	uart_receiver_buffer_t buffers[UART_RECEIVER_MAX_BUFFERS];
    uart_receiver_buffer_t buffer_active;
    uart_receiver_buffer_t buffer_pending;

    uint16_t buffer_index_active;
    uint16_t buffer_index_pending;

    uint16_t old_pos;

	UART_HandleTypeDef* huart;
} uart_receiver_t;


uint8_t UartRcvr_init(uart_receiver_t *uart_rcvr, UART_HandleTypeDef *huart);
void UartRcvr_deinit(uart_receiver_t *uart_rcvr);

uint8_t UartRcvr_available(uart_receiver_t *uart_rcvr);
uint16_t UartRcvr_get_input(uart_receiver_t *uart_rcvr, uint8_t* str, uint16_t max_len);

void UartRcvr_it_complete(uart_receiver_t *uart_rcvr, uint16_t size);

void UartRcvr_it_swap(uart_receiver_t *uart_rcvr);
void UartRcvr_it_process(uart_receiver_t *uart_rcvr, uint16_t offset, uint16_t size);
void UartRcvr_it_trigger(uart_receiver_t *uart_rcvr, uint16_t size);





#endif /* UART_RECEIVER__H_ */










