/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

#include <string.h>

/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN Private defines */

#define UART_RX_LINE_BUFFER_SIZE 64
#define UART_RX_BUFFER_LINES 8
#define UART_TX_TIMEOUT 100

typedef struct {
	uint8_t data[UART_RX_LINE_BUFFER_SIZE];
	uint16_t index;
	uint8_t available;
} uart_line_buffer_t;

typedef struct {
	uart_line_buffer_t lines[UART_RX_BUFFER_LINES];
	uint8_t active_line;
	uint8_t last_char;

	uint8_t trim_newlines;

	UART_HandleTypeDef* huart;
} uart_data_t;

extern uart_data_t uart1_cfg;
extern uart_data_t uart2_cfg;

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

/* USER CODE BEGIN Prototypes */

HAL_StatusTypeDef uart_print(UART_HandleTypeDef* p_huart, const char* str);
uint16_t uart_available(UART_HandleTypeDef* p_huart);
uint16_t uart_get_input(UART_HandleTypeDef* p_huart, uint8_t* str, uint8_t max_len);
void uart_set_baudrate(UART_HandleTypeDef* p_huart, uint32_t baud_rate);

void uart_it_idle(uart_data_t* uart_rx);
void uart_it_complete(uart_data_t* uart_rx, uint16_t size);


HAL_StatusTypeDef prints(const char* str);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

