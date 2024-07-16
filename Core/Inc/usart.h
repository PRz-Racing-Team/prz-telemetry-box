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

/* USER CODE BEGIN Private defines */

#define UART1_RX_BUFFER_SIZE 255
#define UART1_RX_TIMEOUT 100
#define UART1_TX_TIMEOUT 1000


typedef struct {
	uint16_t buffer_size;
	uint16_t index;
	uint8_t available;
	uint8_t trim_newlines;

	uint8_t data[UART1_RX_BUFFER_SIZE];
	UART_HandleTypeDef* huart;
} uart_data_t;

extern uart_data_t uart1_rx;

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);

/* USER CODE BEGIN Prototypes */

HAL_StatusTypeDef uart_print(UART_HandleTypeDef* p_huart, const char* str);
uint16_t uart_available(UART_HandleTypeDef* p_huart);
uint16_t uart_get_input(UART_HandleTypeDef* p_huart, uint8_t* str, uint8_t max_len);
void uart_it(uart_data_t* uart_rx);

HAL_StatusTypeDef prints(const char* str);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

