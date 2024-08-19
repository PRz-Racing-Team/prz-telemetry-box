/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "can.h"
#include "dma.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

//#include "../frame_collector/frame_collector.h"
#include "../circular_buffers/can_frames_cb.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uart_receiver_t usart1_rcvr;
uart_receiver_t usart2_rcvr;
uart_receiver_t uart7_rcvr;

gsm_t gsm;

#define RX_BUF_SIZE 255
  	uint8_t rx_buf[RX_BUF_SIZE];
  	uint16_t rx_index = 0;

#define STR_BUF_SIZE 512
	uint8_t str[STR_BUF_SIZE];
	uint16_t len;

CAN_RxHeaderTypeDef can_rx_header;
uint8_t can_rx_data[8];


can_frames_cb_t can_frames_cb = {0};

#define UDP_BUFFER_SIZE 500
#define UDP_BUFFER_COUNT 10

typedef struct {
	uint8_t data[UDP_BUFFER_SIZE];
	uint16_t len;
} udp_buffer_t;

udp_buffer_t udp_buffer[UDP_BUFFER_COUNT] = {0};
uint8_t udp_buffer_head = 0;
uint8_t udp_buffer_tail = 0;


//frame_collector_t fc;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_UART7_Init();
  MX_CAN1_Init();
  MX_CAN2_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

	UartRcvr_init(&usart1_rcvr, &huart1);
	UartRcvr_init(&usart2_rcvr, &huart2);
	UartRcvr_init(&uart7_rcvr, &huart7);

	prints("PRz Telemetry Box 4.0\r\n");

	HAL_Delay(1000);

//	if (FC_Init(&fc) == 0) {
//		prints("FC_Init failed\r\n");
//		Error_Handler();
//	}

	FT_ERR ft_err = FT_InitCustom(&ft, &huart1, &htim2, 10000);
	if(ft_err != FT_OK)
	{
		prints("FT_InitCustom failed\r\n");
		Error_Handler();
	}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	FT_PrintConfiguration(ft, (char*)str, 2048);
	prints((char*) str);

	GSM_ERR gsm_err = GSM_Init(&gsm, ft, &usart2_rcvr, &usart1_rcvr);
	if (gsm_err != GSM_OK) {
		prints("GSM_Init failed\r\n");
		Error_Handler();
	}

	CAN_TxHeaderTypeDef can_tx_header;
	uint8_t can_tx_data[8];

	can_tx_header.StdId = 0x123;
	can_tx_header.ExtId = 0;
	can_tx_header.DLC = 1;
	can_tx_header.IDE = CAN_ID_STD;
	can_tx_header.RTR = CAN_RTR_DATA;
	can_tx_header.TransmitGlobalTime = DISABLE;

	can_tx_data[0] = 0xAA;

	uint32_t can_tx_mailbox1, can_tx_mailbox2;

	prints("Sending CAN messages\r\n");
	HAL_CAN_AddTxMessage(&hcan1, &can_tx_header, can_tx_data, &can_tx_mailbox1);
	HAL_CAN_AddTxMessage(&hcan2, &can_tx_header, can_tx_data, &can_tx_mailbox2);





	// let the GSM module initialize
	while ((gsm_err = GSM_Feed(&gsm)) != GSM_IDLE) {
		if (gsm_err == GSM_NOT_INITIALIZED || gsm_err == GSM_INVALID_ARGUMENT) {
			prints("GSM_Feed failed\r\n");
		}
	}

	can_frame_t can_frame;
	while (1)
	{
		while (CFCB_Pop(&can_frames_cb, &can_frame) == 1)
		{
			//FC_FeedFrame(&fc, can_frame.id, can_frame.len, can_frame.data);
			if (udp_buffer[udp_buffer_head].len + can_frame.len + 3 >= UDP_BUFFER_SIZE) {
				if ((udp_buffer_head + 1) % UDP_BUFFER_COUNT == udp_buffer_tail)
				{
					//prints("UDP buffer full\r\n");
					break;
				}
				udp_buffer_head = (udp_buffer_head + 1) % UDP_BUFFER_COUNT;
				udp_buffer[udp_buffer_head].len = 0;
			}
			else
			{
				udp_buffer[udp_buffer_head].data[udp_buffer[udp_buffer_head].len++] = can_frame.id & 0xFF;
				udp_buffer[udp_buffer_head].data[udp_buffer[udp_buffer_head].len++] = can_frame.len & 0xFF;
				memcpy(udp_buffer[udp_buffer_head].data + udp_buffer[udp_buffer_head].len, can_frame.data, can_frame.len);
				udp_buffer[udp_buffer_head].len += can_frame.len;
			}
		}

		gsm_err = GSM_Feed(&gsm);
		if(gsm_err == GSM_IDLE)
		{
			if (udp_buffer_head != udp_buffer_tail)
			{
				GSM_SendUDP(&gsm, udp_buffer[udp_buffer_tail].data, udp_buffer[udp_buffer_tail].len);
				udp_buffer_tail = (udp_buffer_tail + 1) % UDP_BUFFER_COUNT;
			}
			//GSM_SendUDPString(&gsm, "ABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZ");
		}

		if(UartRcvr_available(&usart1_rcvr))
		{
			len = UartRcvr_get_input(&usart1_rcvr, rx_buf + rx_index, RX_BUF_SIZE - rx_index - 1);
			if(len == 0) continue;
			rx_index += len;
			if (rx_buf[rx_index - 1] == '\n' || rx_buf[rx_index - 1] == '\r') {
				rx_buf[rx_index - 1] = 0;
				rx_index = 0;
			}
			else continue;

			prints("U1: ");
			prints((char*) rx_buf);
			prints("\r\n");

			uint32_t baud_rate = atoi((char*) rx_buf);
			if (baud_rate == 115200 || baud_rate == 921600)
			{
				snprintf((char*) str, STR_BUF_SIZE, "Changing baud rate to %ld\r\n", baud_rate);
				prints((char*) str);
				UartRcvr_set_baud_rate(&usart2_rcvr, baud_rate);
			}
			else
			{
				//while ((gsm_err = GSM_Feed(&gsm)) != GSM_IDLE) {}

				snprintf((char*) str, STR_BUF_SIZE, "%s\r\n", rx_buf);
				GSM_at(&gsm, (const char*)str, 0, 1000);
			}

		}
//		while(UartRcvr_available(&usart2_rcvr))
//		{
//			len = UartRcvr_get_input(&usart2_rcvr, str, STR_BUF_SIZE - 1);
//			if (len == 0) continue;
//			str[len] = 0;
//			prints("GSM: ");
//			prints((char*) str);
//			prints("\r\n");
//		}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 90;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &can_rx_header, can_rx_data) == HAL_OK)
	{
		if (CFCB_IsFull(&can_frames_cb))
		{
			//prints("CAN frames buffer full\r\n");
			return;
		}

		if(CFCB_Push(&can_frames_cb, can_rx_header.StdId, can_rx_header.DLC, can_rx_data) == 0)
		{
			//prints("GCB_Push failed\r\n");
		}
	}

}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if(huart->Instance == USART1)
    {
    	UartRcvr_it_trigger(&usart1_rcvr, size);
    	if(huart->RxEventType == HAL_UART_RXEVENT_IDLE)
    	{
    		UartRcvr_it_swap(&usart1_rcvr);
    	}
    }
    else if (huart->Instance == USART2)
    {
    	UartRcvr_it_trigger(&usart2_rcvr, size);
    	if(huart->RxEventType == HAL_UART_RXEVENT_IDLE)
    	{
    		UartRcvr_it_swap(&usart2_rcvr);
    	}
	}
    else if (huart->Instance == UART7) {
		UartRcvr_it_trigger(&uart7_rcvr, size);
		if (huart->RxEventType == HAL_UART_RXEVENT_IDLE)
		{
			UartRcvr_it_swap(&uart7_rcvr);
		}
	}
}


void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		UartRcvr_it_error(&usart1_rcvr);
	} else if (huart->Instance == USART2) {
		UartRcvr_it_error(&usart2_rcvr);
	} else if (huart->Instance == UART7) {
		UartRcvr_it_error(&uart7_rcvr);
	}
}


HAL_StatusTypeDef prints(const char* str)
{
	return UartRcvr_print(&usart1_rcvr, str);
}




/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	prints("Error_Handler\r\n");
	HAL_Delay(10000);
	// restart the MCU
	NVIC_SystemReset();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
