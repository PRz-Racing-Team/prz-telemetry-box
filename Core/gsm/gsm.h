/*
 * gsm.h
 *
 *  Created on: Jul 16, 2024
 *      Author: Dariusz Strojny @ReijiY
 */

#ifndef GSM__H_
#define GSM__H_

#include "../fancy_timer/fancy_timer.h"
#include "../uart_receiver/uart_receiver.h"

#include "gsm_defines_errors.h"

#define GSM_TIMER_CSQ_INTERVAL 50000 // 5s
#define GSM_TIMER_CSQ_PRIORITY 10

#define GSM_RX_BUFFER_SIZE 512
#define GSM_LINE_BUFFER_SIZE 128

#define GSM_TIMER_TIME_COUNTER_INTERVAL 				1 		// 0.1 ms -> 10000 Hz
#define GSM_TIMER_TIME_COUNTER_PRIORITY 				100

#define GSM_TIMER_TIMEOUT_INTERVAL 						10000 	// 1 s -> 1 Hz
#define GSM_TIMER_TIMEOUT_PRIORITY 						90

#define GSM_TIMER_DETECT_INTERVAL 						5000 	// 0.5 s -> 2 Hz
#define GSM_TIMER_DETECT_PRIORITY 						80

typedef enum {
	GSM_BAUD_RATE_115200,
	GSM_BAUD_RATE_921600
} gsm_baud_rate_t;

typedef struct {
	uint16_t time_counter;
	uint16_t timeout;
	uint16_t detect;
} gsm_ft_timers_t;

typedef struct {
	gsm_baud_rate_t baud_rate;
} gsm_config_t;

//typedef struct {
//	uint8_t ok;
//	uint8_t error;
//	uint8_t unknown;
//};

typedef struct {
	// GSM status
	uint8_t detected;
	uint8_t detecting;
	uint8_t initialized;
	uint8_t initializing;

	// response status
	uint8_t timeout;
	uint16_t timeout_count;
	uint8_t response_expected;
	uint8_t response_available;
	uint16_t response_lines;
	uint8_t response_awaiting;
	uint8_t response_ok;
	uint8_t response_error;
	uint8_t response_unknown;
	uint8_t response_command;

} gsm_flags_t;

typedef struct {
	gsm_flags_t flags;
	gsm_config_t config;
	gsm_ft_timers_t timers;

	uint16_t timer_id;
	uint32_t timer_trigger_count;

	uint8_t rx_buf[GSM_RX_BUFFER_SIZE];
	uint16_t rx_buf_len;
	uint16_t rx_buf_processed;

	uint8_t line_buf[GSM_LINE_BUFFER_SIZE];
	uint16_t line_buf_len;

	uint64_t time;

	FT_base *ft;
	uart_receiver_t *uart_rcvr_gsm;
	uart_receiver_t *uart_rcvr_debug;
} gsm_t;

GSM_ERR GSM_cmd(gsm_t *gsm, const uint8_t* cmd, uint16_t cmd_len);
GSM_ERR GSM_str(gsm_t *gsm, const char* str);
GSM_ERR GSM_Prints(gsm_t *gsm, const char* str);

GSM_ERR GSM_at(gsm_t *gsm, const char* at_cmd, uint8_t response_expected, uint32_t timeout_ms);

GSM_ERR GSM_Feed(gsm_t *gsm);

GSM_ERR GSM_AwaitResponse(gsm_t *gsm);

GSM_ERR GSM_ProcessInput(gsm_t *gsm);

GSM_ERR GSM_InitModem(gsm_t *gsm);

GSM_ERR GSM_SetBaudRate(gsm_t *gsm, gsm_baud_rate_t baud_rate);
GSM_ERR GSM_ChangeBaudRate(gsm_t *gsm);

GSM_ERR GSM_Init(gsm_t *gsm, FT_base *ft, uart_receiver_t *uart_rcvr_gsm, uart_receiver_t *uart_rcvr_debug);
GSM_ERR GSM_DeInit(gsm_t *gsm);

GSM_ERR GSM_ClearResponse(gsm_t *gsm);

GSM_ERR GSM_GetLine(gsm_t *gsm);

#endif /* GSM__H_ */
