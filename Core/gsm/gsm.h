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

#define GSM_RX_BUFFER_SIZE 								512
#define GSM_TX_BUFFER_SIZE 								512
#define GSM_LINE_BUFFER_SIZE 							128
#define GSM_COMMAND_BUFFER_SIZE 						256
#define GSM_COMMAND_MAX_PARAMETERS 						32
#define GSM_SMS_BUFFER_SIZE 							128

#define GSM_TIMER_TIMEOUT_INTERVAL 						10000 	// 1 s -> 1 Hz
#define GSM_TIMER_TIMEOUT_PRIORITY 						90

#define GSM_TIMER_DETECT_INTERVAL 						5000 	// 0.5 s -> 2 Hz
#define GSM_TIMER_DETECT_PRIORITY 						80


typedef enum {
	GSM_BAUD_RATE_115200,
	GSM_BAUD_RATE_921600
} gsm_baud_rate_t;

typedef struct {
//	uint16_t time_counter;
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
	uint8_t timeout;

	uint8_t expected;
	uint8_t available;
	uint16_t lines;
	uint8_t awaiting;

	uint8_t has_ok;
	uint8_t has_error;
	uint8_t has_command;
	uint8_t has_input_request;
	uint8_t has_unknown;

	uint8_t command_buf[GSM_COMMAND_BUFFER_SIZE];
	uint16_t command_buf_len;
	uint16_t command_buf_name_len;

	uint16_t command_buf_parameters_pos[GSM_COMMAND_MAX_PARAMETERS];
	uint16_t command_buf_parameters_len[GSM_COMMAND_MAX_PARAMETERS];
	uint8_t command_buf_parameters_count;


} gsm_flags_response_t;

typedef struct {
	uint8_t detected;
	uint8_t detecting;

	uint8_t initialized;
	uint8_t initializing;

	uint8_t network_opened;
	uint8_t network_opening;

	uint8_t connection_opened;
	uint8_t connection_opening;

	uint8_t data_sending;
	uint8_t data_prepared;

	uint8_t line_complete;
	uint8_t receiving_sms;

	uint8_t sms_available;
	int32_t sms_pdu_len;

	gsm_flags_response_t response;
	uint16_t timeout_count;

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

	uint8_t tx_buf[GSM_TX_BUFFER_SIZE];
	uint16_t tx_buf_len;

	uint8_t line_buf[GSM_LINE_BUFFER_SIZE];
	uint16_t line_buf_len;

	uint8_t sms_buf[GSM_SMS_BUFFER_SIZE];
	uint16_t sms_buf_len;

	//uint64_t time;

	FT_base *ft;
	uart_receiver_t *uart_rcvr_gsm;
	uart_receiver_t *uart_rcvr_debug;
} gsm_t;

GSM_ERR GSM_cmd(gsm_t *gsm, const uint8_t* cmd, uint16_t cmd_len);
GSM_ERR GSM_str(gsm_t *gsm, const char* str);
GSM_ERR GSM_PrintsData(gsm_t *gsm, const uint8_t *data, uint16_t len);
GSM_ERR GSM_Prints(gsm_t *gsm, const char* str);

GSM_ERR GSM_atData(gsm_t *gsm, const uint8_t* at_cmd, uint16_t len, uint8_t response_expected, uint32_t timeout_ms);
GSM_ERR GSM_at(gsm_t *gsm, const char* at_cmd, uint8_t response_expected, uint32_t timeout_ms);

GSM_ERR GSM_Feed(gsm_t *gsm);

GSM_ERR GSM_AwaitResponse(gsm_t *gsm);

GSM_ERR GSM_ProcessInput(gsm_t *gsm);

GSM_ERR GSM_InitModem(gsm_t *gsm);
GSM_ERR GSM_OpenNetwork(gsm_t *gsm);
GSM_ERR GSM_OpenConnection(gsm_t *gsm);

GSM_ERR GSM_SetBaudRate(gsm_t *gsm, gsm_baud_rate_t baud_rate);
GSM_ERR GSM_ChangeBaudRate(gsm_t *gsm);

GSM_ERR GSM_Init(gsm_t *gsm, FT_base *ft, uart_receiver_t *uart_rcvr_gsm, uart_receiver_t *uart_rcvr_debug);
GSM_ERR GSM_DeInit(gsm_t *gsm);

GSM_ERR GSM_ClearResponse(gsm_t *gsm);

GSM_ERR GSM_GetLine(gsm_t *gsm);
GSM_ERR GSM_DecodePDU(gsm_t *gsm, const char* pdu, uint16_t pdu_len);
GSM_ERR GSM_ProcessCommand(gsm_t *gsm, const char* cmd, uint16_t cmd_len);

GSM_ERR GSM_CommandCompareName(gsm_t *gsm, const char* str);
GSM_ERR GSM_CommandCompareParameter(gsm_t *gsm, uint16_t param_i, const char* str);
GSM_ERR GSM_CommandGetParameter(gsm_t *gsm, uint16_t param_i, char* buf, uint16_t buf_len, uint16_t* param_len);
GSM_ERR GSM_CommandGetParameterInt(gsm_t *gsm, uint16_t param_i, int32_t* result);
GSM_ERR GSM_CommandGetParameterFloat(gsm_t *gsm, uint16_t param_i, float* result);

GSM_ERR GSM_SendUDP(gsm_t *gsm, const uint8_t* data, uint16_t len);
GSM_ERR GSM_SendUDPString(gsm_t *gsm, const char* str);

#endif /* GSM__H_ */
