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

#define UDP_BUFFER_SIZE 1490
#define UDP_BUFFER_COUNT 5

typedef struct {
	uint8_t data[UDP_BUFFER_SIZE + 1];
	uint16_t len;
} udp_buffer_t;

udp_buffer_t udp_buffer[UDP_BUFFER_COUNT] = {0};
uint8_t udp_buffer_head = 0;
uint8_t udp_buffer_tail = 0;


//frame_collector_t fc;


// TODO: this is for tests. Move it to a separate file if it works

static const uint16_t crctab[256] = {
    0x0000U,0x365EU,0x6CBCU,0x5AE2U,0xD978U,0xEF26U,0xB5C4U,0x839AU,
    0xFF89U,0xC9D7U,0x9335U,0xA56BU,0x26F1U,0x10AFU,0x4A4DU,0x7C13U,
    0xB26BU,0x8435U,0xDED7U,0xE889U,0x6B13U,0x5D4DU,0x07AFU,0x31F1U,
    0x4DE2U,0x7BBCU,0x215EU,0x1700U,0x949AU,0xA2C4U,0xF826U,0xCE78U,
    0x29AFU,0x1FF1U,0x4513U,0x734DU,0xF0D7U,0xC689U,0x9C6BU,0xAA35U,
    0xD626U,0xE078U,0xBA9AU,0x8CC4U,0x0F5EU,0x3900U,0x63E2U,0x55BCU,
    0x9BC4U,0xAD9AU,0xF778U,0xC126U,0x42BCU,0x74E2U,0x2E00U,0x185EU,
    0x644DU,0x5213U,0x08F1U,0x3EAFU,0xBD35U,0x8B6BU,0xD189U,0xE7D7U,
    0x535EU,0x6500U,0x3FE2U,0x09BCU,0x8A26U,0xBC78U,0xE69AU,0xD0C4U,
    0xACD7U,0x9A89U,0xC06BU,0xF635U,0x75AFU,0x43F1U,0x1913U,0x2F4DU,
    0xE135U,0xD76BU,0x8D89U,0xBBD7U,0x384DU,0x0E13U,0x54F1U,0x62AFU,
    0x1EBCU,0x28E2U,0x7200U,0x445EU,0xC7C4U,0xF19AU,0xAB78U,0x9D26U,
    0x7AF1U,0x4CAFU,0x164DU,0x2013U,0xA389U,0x95D7U,0xCF35U,0xF96BU,
    0x8578U,0xB326U,0xE9C4U,0xDF9AU,0x5C00U,0x6A5EU,0x30BCU,0x06E2U,
    0xC89AU,0xFEC4U,0xA426U,0x9278U,0x11E2U,0x27BCU,0x7D5EU,0x4B00U,
    0x3713U,0x014DU,0x5BAFU,0x6DF1U,0xEE6BU,0xD835U,0x82D7U,0xB489U,
    0xA6BCU,0x90E2U,0xCA00U,0xFC5EU,0x7FC4U,0x499AU,0x1378U,0x2526U,
    0x5935U,0x6F6BU,0x3589U,0x03D7U,0x804DU,0xB613U,0xECF1U,0xDAAFU,
    0x14D7U,0x2289U,0x786BU,0x4E35U,0xCDAFU,0xFBF1U,0xA113U,0x974DU,
    0xEB5EU,0xDD00U,0x87E2U,0xB1BCU,0x3226U,0x0478U,0x5E9AU,0x68C4U,
    0x8F13U,0xB94DU,0xE3AFU,0xD5F1U,0x566BU,0x6035U,0x3AD7U,0x0C89U,
    0x709AU,0x46C4U,0x1C26U,0x2A78U,0xA9E2U,0x9FBCU,0xC55EU,0xF300U,
    0x3D78U,0x0B26U,0x51C4U,0x679AU,0xE400U,0xD25EU,0x88BCU,0xBEE2U,
    0xC2F1U,0xF4AFU,0xAE4DU,0x9813U,0x1B89U,0x2DD7U,0x7735U,0x416BU,
    0xF5E2U,0xC3BCU,0x995EU,0xAF00U,0x2C9AU,0x1AC4U,0x4026U,0x7678U,
    0x0A6BU,0x3C35U,0x66D7U,0x5089U,0xD313U,0xE54DU,0xBFAFU,0x89F1U,
    0x4789U,0x71D7U,0x2B35U,0x1D6BU,0x9EF1U,0xA8AFU,0xF24DU,0xC413U,
    0xB800U,0x8E5EU,0xD4BCU,0xE2E2U,0x6178U,0x5726U,0x0DC4U,0x3B9AU,
    0xDC4DU,0xEA13U,0xB0F1U,0x86AFU,0x0535U,0x336BU,0x6989U,0x5FD7U,
    0x23C4U,0x159AU,0x4F78U,0x7926U,0xFABCU,0xCCE2U,0x9600U,0xA05EU,
    0x6E26U,0x5878U,0x029AU,0x34C4U,0xB75EU,0x8100U,0xDBE2U,0xEDBCU,
    0x91AFU,0xA7F1U,0xFD13U,0xCB4DU,0x48D7U,0x7E89U,0x246BU,0x1235U,
};

// Automatically generated CRC function
// polynomial: 0x13D65, bit reverse algorithm
uint16_t crc16(uint8_t *data, int len)
{


    uint16_t crc = 0xFFFF;
    crc = crc ^ 0xFFFFU;
    while (len > 0)
    {
        crc = crctab[*data ^ (uint8_t)crc] ^ (crc >> 8);
        data++;
        len--;
    }
    crc = crc ^ 0xFFFFU;
    return crc;
}



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

	prints("PRz Telemetry Box 4.1\r\n");

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

	GSM_ERR gsm_err = GSM_Init(&gsm, ft, &usart2_rcvr, &usart1_rcvr, &uart7_rcvr);
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
	uint8_t data_frame[16];
	while (1)
	{
		gsm_err = GSM_Feed(&gsm);
		while (CFCB_Pop(&can_frames_cb, &can_frame) == 1)
		{
			if (can_frame.len > 8)
			{
				prints("\r\n! can_frame.len > 8 !\r\n");
				continue;
			}

			if(gsm_err == GSM_IDLE)
			{
				data_frame[0] = 0xCF;
				data_frame[1] = can_frame.len + 6;
				uint16_t buffer_size = CFCB_GetSize(&can_frames_cb);
				data_frame[2] = buffer_size & 0xFF;
				data_frame[3] = (buffer_size >> 8) & 0xFF;
				data_frame[4] = can_frame.id & 0xFF;
				data_frame[5] = (can_frame.id >> 8) & 0xFF;
				memcpy(data_frame + 6, can_frame.data, can_frame.len);
				uint16_t crc = crc16(data_frame, 6 + can_frame.len);
				data_frame[6 + can_frame.len] = crc & 0xFF;
				data_frame[7 + can_frame.len] = (crc >> 8) & 0xFF;

				GSM_SendTCP(&gsm, data_frame, can_frame.len + 8);
			}

						//GSM_SendTCPString(&gsm, "ABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZ");

			//FC_FeedFrame(&fc, can_frame.id, can_frame.len, can_frame.data);
//			if (udp_buffer[udp_buffer_head].len + can_frame.len + 3 >= UDP_BUFFER_SIZE) {
//				if ((udp_buffer_head + 1) % UDP_BUFFER_COUNT == udp_buffer_tail)
//				{
//					prints("TCP buffer full\r\n");
//					break;
//				}
//				udp_buffer_head = (udp_buffer_head + 1) % UDP_BUFFER_COUNT;
//				udp_buffer[udp_buffer_head].len = 0;
//			}
//			else
//			{
//				udp_buffer[udp_buffer_head].data[udp_buffer[udp_buffer_head].len++] = can_frame.id & 0xFF;
//				udp_buffer[udp_buffer_head].data[udp_buffer[udp_buffer_head].len++] = can_frame.len & 0xFF;
//				memcpy(udp_buffer[udp_buffer_head].data + udp_buffer[udp_buffer_head].len, can_frame.data, can_frame.len);
//				udp_buffer[udp_buffer_head].len += can_frame.len;
//			}
		}

//		if(gsm_err == GSM_IDLE)
//		{
//			if (udp_buffer_head != udp_buffer_tail)
//			{
//				GSM_SendTCP(&gsm, udp_buffer[udp_buffer_tail].data, udp_buffer[udp_buffer_tail].len);
//				udp_buffer_tail = (udp_buffer_tail + 1) % UDP_BUFFER_COUNT;
//			}
//			uint8_t frame[12];
//			frame[0] = 0xCF;
//			frame[1] = 10;
//			frame[2] = frame_counter & 0xFF;
//			frame[3] = (frame_counter >> 8) & 0xFF;
//			frame[4] = (frame_counter >> 16) & 0xFF;
//			frame[5] = (frame_counter >> 24) & 0xFF;
//			frame[6] = 0xA5;
//			frame[7] = 0x5A;
//			frame[8] = 'h';
//			frame[9] = 'e';
//			uint16_t crc = crc16(frame, 10);
//			frame[10] = crc & 0xFF;
//			frame[11] = (crc >> 8) & 0xFF;
//
//			GSM_SendTCP(&gsm, frame, 12);
//
//			frame_counter++;

			//GSM_SendTCPString(&gsm, "ABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPRSTUVWXYZ");
//		}
//		else {
//			char buf[20];
//			snprintf(buf, 20, "%04X\r\n", gsm_err);
//			GSM_Prints(&gsm, buf);
//		}

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

//			uint32_t baud_rate = atoi((char*) rx_buf);
//			if (baud_rate == 115200 || baud_rate == 921600)
//			{
//				snprintf((char*) str, STR_BUF_SIZE, "Changing baud rate to %ld\r\n", baud_rate);
//				prints((char*) str);
//				UartRcvr_set_baud_rate(&usart2_rcvr, baud_rate);
//			}
//			else
//			{
				//while ((gsm_err = GSM_Feed(&gsm)) != GSM_IDLE) {}

			snprintf((char*) str, STR_BUF_SIZE, "%s\r\n", rx_buf);
			GSM_at(&gsm, (const char*)str, 0, 1000);
//			}

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

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
  RCC_OscInitStruct.PLL.PLLN = 180;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &can_rx_header, can_rx_data) == HAL_OK)
	{
		can_frames_cb.counter++;

		if(CFCB_Push(&can_frames_cb, can_rx_header.StdId, can_rx_header.DLC, can_rx_data) == 0)
		{
			prints(".");
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
