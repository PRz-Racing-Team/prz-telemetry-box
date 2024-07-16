/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

uart_data_t uart1_rx = {
	.buffer_size = UART1_RX_BUFFER_SIZE,
	.index = 0,
	.available = 0,
	.trim_newlines = 0,
	.huart = &huart1
};

/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 921600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

	HAL_UART_Receive_IT(&huart1, uart1_rx.data + uart1_rx.index, 1);
  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

HAL_StatusTypeDef uart_print(UART_HandleTypeDef* p_huart, const char* str)
{
	return HAL_UART_Transmit(p_huart, (uint8_t*)str, strlen(str), UART1_TX_TIMEOUT);
}

uint16_t uart_available(UART_HandleTypeDef* p_huart)
{
	if (p_huart->Instance == USART1) {
		return uart1_rx.available;
	}
	return 0;
}

uint16_t uart_get_input(UART_HandleTypeDef* p_huart, uint8_t* str, uint8_t max_len)
{
	if (p_huart->Instance == USART1 && uart1_rx.available) {
		uint16_t len = 0;
		if(str != NULL)
		{
			len = uart1_rx.index;
			if (len > max_len) len = max_len;
			memcpy(str, uart1_rx.data, len);
			str[len] = '\0';
		}
		uart1_rx.index = 0;
		uart1_rx.available = 0;
		HAL_UART_Receive_IT(p_huart, uart1_rx.data + uart1_rx.index, 1);
		return len;
	}
	return 0;
}

void uart_it(uart_data_t* uart_rx)
{

	if(uart_rx->index < uart_rx->buffer_size - 1)
	{
		uart_rx->index++;
		if(uart_rx->data[uart_rx->index - 1] == '\n'
				|| uart_rx->data[uart_rx->index - 1] == '\r'
				|| uart_rx->index == uart_rx->buffer_size - 1)
		{
			// trim trailing newlines and carriage returns
			while (uart_rx->trim_newlines && uart_rx->index != 0 && (uart_rx->data[uart_rx->index - 1] == '\n' || uart_rx->data[uart_rx->index - 1] == '\r'))
			{
				uart_rx->index--;
			}
			uart_rx->data[uart_rx->index] = '\0';
			uart_rx->available = 1;
		}
		else HAL_UART_Receive_IT(uart_rx->huart, uart_rx->data + uart_rx->index, 1);
	}
	else uart_rx->available = 1;
}

HAL_StatusTypeDef prints(const char* str)
{
	return uart_print(&huart1, str);
}

/* USER CODE END 1 */
