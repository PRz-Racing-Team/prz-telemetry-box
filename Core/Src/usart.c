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

uart_data_t uart1_cfg;
uart_data_t uart2_cfg;

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_rx;

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
	memset(&uart1_cfg, 0, sizeof(uart_data_t));
	uart1_cfg.huart = &huart1;
	uart1_cfg.trim_newlines = 1;

	uart_line_buffer_t* line = uart1_cfg.lines + uart1_cfg.active_line;
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, line->data, UART_RX_LINE_BUFFER_SIZE);
  /* USER CODE END USART1_Init 2 */

}
/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */
	memset(&uart2_cfg, 0, sizeof(uart_data_t));
	uart2_cfg.huart = &huart1;
	uart2_cfg.trim_newlines = 1;

	uart_line_buffer_t* line = uart2_cfg.lines + uart2_cfg.active_line;
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, line->data, UART_RX_LINE_BUFFER_SIZE);
  /* USER CODE END USART2_Init 2 */

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

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA2_Stream2;
    hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_NORMAL;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);

  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Stream5;
    hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
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

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5|GPIO_PIN_6);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

HAL_StatusTypeDef uart_print(UART_HandleTypeDef* p_huart, const char* str)
{
	return HAL_UART_Transmit(p_huart, (uint8_t*)str, strlen(str), UART_TX_TIMEOUT);
}

uint16_t uart_available(UART_HandleTypeDef* p_huart)
{
	uart_data_t* uart_rx = NULL;
	if (p_huart->Instance == USART1)
	{
		uart_rx = &uart1_cfg;
	} else if (p_huart->Instance == USART2)
	{
		uart_rx = &uart2_cfg;
	}
	if (uart_rx == NULL) return 0;

	for (uint8_t i = 0; i < UART_RX_BUFFER_LINES; i++)
	{
		if (uart_rx->lines[i].available)
		{
			return 1;
		}
	}

	return 0;
}

uint16_t uart_get_input(UART_HandleTypeDef* p_huart, uint8_t* str, uint8_t max_len)
{
	uart_data_t *uart_rx = NULL;
	if (p_huart->Instance == USART1)
	{
		uart_rx = &uart1_cfg;
	} else if (p_huart->Instance == USART2)
	{
		uart_rx = &uart2_cfg;
	}
	if (uart_rx == NULL) return 0;


	for (uart_line_buffer_t *line = uart_rx->lines; line < uart_rx->lines + UART_RX_BUFFER_LINES; line++)
	{
		if (line->available)
		{
			uint16_t len = line->index;
			if (len > max_len) len = max_len;
			memcpy(str, line->data, len);
			str[len] = '\0';
			line->available = 0;
			line->index = 0;
			return len;
		}
	}

	return 0;
}

void uart_set_baudrate(UART_HandleTypeDef* huart, uint32_t baud_rate)
{
	uint32_t pclk;
	if (huart->Instance == USART1 || huart->Instance == USART6) {
		// USART1 and USART6 are on APB2
		pclk = HAL_RCC_GetPCLK2Freq();
	} else {
		// USART2, USART3, UART4, UART5 are on APB1
		pclk = HAL_RCC_GetPCLK1Freq();
	}
    uint32_t usartdiv = (pclk + (baud_rate / 2)) / baud_rate;

    huart->Instance->CR1 &= ~(USART_CR1_UE);
	__DSB(); // barrier
	(void)(huart->Instance->CR1); // read-back
	__DMB();

    huart->Instance->BRR = usartdiv;
    __DSB();
    (void)(huart->Instance->BRR);
    __DMB();

    huart->Instance->CR1 |= USART_CR1_UE;
	__DSB();
	(void)(huart->Instance->CR1);
	__DMB();
}

void uart_it_idle(uart_data_t* uart_rx)
{
	uart_line_buffer_t *line = uart_rx->lines + uart_rx->active_line;
	if (line->index == 0) return;
	line->index++;
	line->data[line->index] = '\0';
	line->available = 1;
	if (++uart_rx->active_line == UART_RX_BUFFER_LINES) {
		uart_rx->active_line = 0;
		line = uart_rx->lines + uart_rx->active_line;
		line->index = 0;
		line->available = 0;
	}
}

void uart_it_complete(uart_data_t* uart_rx, uint16_t size)
{
	if(size == 0) return;

	uart_line_buffer_t* line = uart_rx->lines + uart_rx->active_line;

	line->index = size;

	uart_rx->last_char = line->data[line->index];

	line->data[line->index] = '\0';
	line->available = 1;
	if (++uart_rx->active_line >= UART_RX_BUFFER_LINES)
	{
		uart_rx->active_line = 0;
		line = uart_rx->lines + uart_rx->active_line;
		line->index = 0;
		line->available = 0;
	}
}

HAL_StatusTypeDef prints(const char* str)
{
	return uart_print(&huart1, str);
}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
	uart_data_t* uart_rx = NULL;
    if(huart->Instance == USART1)
    {
    	uart_rx = &uart1_cfg;
    }
    else if (huart->Instance == USART2)
    {
		uart_rx = &uart2_cfg;
	}
    if (uart_rx == NULL) return;

    uart_it_complete(uart_rx, size);

    uart_line_buffer_t* line = uart_rx->lines + uart_rx->active_line;
    HAL_UARTEx_ReceiveToIdle_DMA(huart, line->data, UART_RX_LINE_BUFFER_SIZE);
}

/* USER CODE END 1 */
