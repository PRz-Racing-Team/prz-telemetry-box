/*
 * gsm.c
 *
 *  Created on: Jul 16, 2024
 *      Author: Dariusz Strojny @ReijiY
 */

#include "gsm.h"

GSM_ERR GSM_cmd(gsm_t *gsm, const uint8_t* cmd, uint16_t cmd_len)
{
	if(gsm == NULL) return GSM_INVALID_ARGUMENT;
	if(gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;
	HAL_StatusTypeDef status = UartRcvr_send(gsm->uart_rcvr_gsm, cmd, cmd_len);
	return status == HAL_OK ? GSM_OK : GSM_HAL_ERR;
}

GSM_ERR GSM_str(gsm_t *gsm, const char* str)
{
	if (gsm == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

	return GSM_cmd(gsm, (const uint8_t*)str, strlen(str));
}

GSM_ERR GSM_Prints(gsm_t *gsm, const char* str)
{
	if (gsm == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->uart_rcvr_debug == NULL) return GSM_OK;

	HAL_StatusTypeDef status = UartRcvr_print(gsm->uart_rcvr_debug, str);
	return status == HAL_OK ? GSM_OK : GSM_HAL_ERR;
}


GSM_ERR GSM_at(gsm_t *gsm, const char* at_cmd, uint8_t response_expected, uint32_t timeout_ms)
{
	if (gsm == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->ft == NULL || gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

	GSM_Prints(gsm, ">> ");
	GSM_Prints(gsm, at_cmd);

	GSM_ClearResponse(gsm);

	if (response_expected == 1)
	{
		gsm->flags.response_expected = 1;
		FT_SetTimerInterval(gsm->ft, gsm->timers.timeout, timeout_ms * 10);
		FT_StartTimer(gsm->ft, gsm->timers.timeout);
	}

	GSM_ERR err = GSM_str(gsm, at_cmd);
	if (err != GSM_OK) return err;

	return GSM_OK;
}



GSM_ERR GSM_Feed(gsm_t *gsm)
{
	if (gsm == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->ft == NULL || gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

	GSM_ERR err = GSM_ProcessInput(gsm);
	if (err != GSM_OK) return err;

	FT_ERR ft_err = FT_Feed(gsm->ft);
	while (ft_err == FT_TRIGGERED && FT_GetTriggeredTimer(gsm->ft, &gsm->timer_id, &gsm->timer_trigger_count) == FT_OK)
	{
		if(gsm->timer_id == gsm->timers.time_counter)
		{
			gsm->time += gsm->timer_trigger_count;
		}
		else if (gsm->timer_id == gsm->timers.timeout)
		{
			GSM_Prints(gsm, "Timeout\r\n");
			gsm->flags.timeout = 1;
			gsm->flags.timeout_count++;

			FT_StopTimer(gsm->ft, gsm->timers.timeout);

			if(timeout_count > 3)
			{
				gsm->flags.detected = 0;
				gsm->flags.initialized = 0;
				FT_StartTimer(gsm->ft, gsm->timers.detect);
			}

		}
		else if (gsm->timer_id == gsm->timers.detect)
		{
			if(gsm->flags.response_awaiting == 1) break;
			GSM_Prints(gsm, "Detect\r\n");
			if (gsm->flags.detected == 0)
			{
				if(gsm->flags.detecting && gsm->flags.timeout == 0 && gsm->flags.response_available == 0) break;
				gsm->flags.detecting = 1;
				snprintf((char*)gsm->line_buf, GSM_LINE_BUFFER_SIZE, "GSM Dtct [%d]\r\n", gsm->flags.timeout_count + 1);
				GSM_Prints(gsm, (char*)gsm->line_buf);
				GSM_ChangeBaudRate(gsm);
				GSM_at(gsm, "\r\nAT\r\n", 1, 1000);
			}
			else {
				FT_StopTimer(gsm->ft, gsm->timers.detect);
			}
		}
		else
		{
			GSM_Prints(gsm, "Unknown timer triggered\r\n");
		}
	}

	if (gsm->flags.detected == 1 && gsm->flags.initialized == 0 && gsm->flags.initializing == 0)
	{
		return GSM_InitModem(gsm);
	}


	return GSM_IDLE;
}

GSM_ERR GSM_AwaitResponse(gsm_t *gsm)
{
	if (gsm == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->ft == NULL || gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

	gsm->flags.response_awaiting = 1;

	while (gsm->flags.response_expected == 1 && gsm->flags.response_available == 0 && gsm->flags.timeout == 0)
	{
		GSM_Feed(gsm);
	}

	if (gsm->flags.timeout)
	{
		return GSM_TIMEOUT;
	}

	gsm->flags.response_expected = 0;
	return GSM_OK;
}

GSM_ERR GSM_ProcessInput(gsm_t *gsm)
{
	if(gsm == NULL) return GSM_INVALID_ARGUMENT;
	if(gsm->ft == NULL || gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

	while (UartRcvr_available(gsm->uart_rcvr_gsm)) {
		gsm->rx_buf_len = UartRcvr_get_input(gsm->uart_rcvr_gsm, gsm->rx_buf, GSM_RX_BUFFER_SIZE);
		if (gsm->rx_buf_len == 0) continue;
		gsm->rx_buf_processed = 0;


		while (GSM_GetLine(gsm) == GSM_OK)
		{
			gsm->flags.response_lines++;

			GSM_Prints(gsm, "<< ");
			GSM_Prints(gsm, (char*)gsm->line_buf);
			GSM_Prints(gsm, "\r\n");

			if (gsm->line_buf_len >= 2 && strncmp((char*)gsm->line_buf, "OK", 2) == 0)
			{
				gsm->flags.response_ok = 1;
				gsm->flags.detected = 1;
				gsm->flags.response_available = 1;
			}
			else if (gsm->line_buf_len >= 5 && strncmp((char*)gsm->line_buf, "ERROR", 5) == 0)
			{
				gsm->flags.response_error = 1;
				gsm->flags.response_available = 1;
			}
			else if (gsm->line_buf_len >= 1 && strncmp((char*)gsm->line_buf, "+", 1) == 0)
			{
				gsm->flags.response_command = 1;
				gsm->flags.response_available = 1;
			}
			else {
				gsm->flags.response_unknown = 1;
			}

		}
	}

	if (gsm->flags.response_available == 1) {
		gsm->flags.timeout_count = 0;
		gsm->flags.timeout = 0;
		FT_StopTimer(gsm->ft, gsm->timers.timeout);
	}

	return GSM_OK;
}

GSM_ERR GSM_InitModem(gsm_t *gsm)
{
	if (gsm == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->ft == NULL || gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

	gsm->flags.initializing = 1;

	GSM_Prints(gsm, "GSM Init\r\n");

	GSM_at(gsm, "ATE0\r\n", 1, 1000);
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+IPR=921600\r\n", 1, 1000);
	GSM_SetBaudRate(gsm, GSM_BAUD_RATE_921600);
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CSCS=\"GSM\"\r\n", 1, 1000);
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CNMI=2,2,0,0,0\r\n", 1, 1000);
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CMGF=1\r\n", 1, 1000);
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CUSD=1\r\n", 1, 1000);
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CGDCONT=1,\"IP\",\"internet\",\"0.0.0.0\",0,0\r\n", 1, 1000);
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CGACT=1,1\r\n", 1, 1000);
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CIPCCFG=3,0,0,0,1,0,500\r\n", 1, 1000);
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CNTP=\"pool.ntp.org\",8\r\n", 1, 1000);
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	gsm->flags.initializing = 0;
	gsm->flags.initialized = 1;

	return GSM_OK;
}

GSM_ERR GSM_SetBaudRate(gsm_t *gsm, gsm_baud_rate_t baud_rate)
{
	if(gsm == NULL) return GSM_INVALID_ARGUMENT;
	if(gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

	uint32_t baud_rate_val = 0;
	switch (baud_rate) {
		case GSM_BAUD_RATE_115200:
			baud_rate_val = 115200;
			GSM_Prints(gsm, "Setting baud rate to 115200\r\n");
			break;
		case GSM_BAUD_RATE_921600:
			baud_rate_val = 921600;
			GSM_Prints(gsm, "Setting baud rate to 921600\r\n");
			break;
		default:
			return GSM_INVALID_ARGUMENT;
	}

	uint8_t status = UartRcvr_set_baud_rate(gsm->uart_rcvr_gsm, baud_rate_val);
	if(status == 0) return GSM_BAUD_RATE_ERR;
	gsm->config.baud_rate = baud_rate;

	return GSM_OK;
}

GSM_ERR GSM_ChangeBaudRate(gsm_t *gsm)
{
	if(gsm == NULL) return GSM_INVALID_ARGUMENT;
	if(gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

	gsm_baud_rate_t new_baud_rate = gsm->config.baud_rate == GSM_BAUD_RATE_115200 ? GSM_BAUD_RATE_921600 : GSM_BAUD_RATE_115200;

	return GSM_SetBaudRate(gsm, new_baud_rate);
}

GSM_ERR GSM_Init(gsm_t *gsm, FT_base *ft, uart_receiver_t *uart_rcvr_gsm, uart_receiver_t *uart_rcvr_debug)
{
	if (gsm == NULL || ft == NULL || uart_rcvr_gsm == NULL) return GSM_INVALID_ARGUMENT;

	gsm->ft = ft;
	gsm->uart_rcvr_gsm = uart_rcvr_gsm;
	gsm->uart_rcvr_debug = uart_rcvr_debug;

	gsm->config.baud_rate = GSM_BAUD_RATE_115200;

	FT_ERR ft_err = FT_NewTimer(ft, GSM_TIMER_TIME_COUNTER_INTERVAL, GSM_TIMER_TIME_COUNTER_PRIORITY, &gsm->timers.time_counter);

	if(ft_err == FT_OK) ft_err = FT_NewTimer(ft, GSM_TIMER_TIMEOUT_INTERVAL, GSM_TIMER_TIMEOUT_PRIORITY, &gsm->timers.timeout);
	FT_StopTimer(ft, gsm->timers.timeout);

	if(ft_err == FT_OK) ft_err = FT_NewTimer(ft, GSM_TIMER_DETECT_INTERVAL, GSM_TIMER_DETECT_PRIORITY, &gsm->timers.detect);

	if (ft_err != FT_OK) {
		GSM_DeInit(gsm);
		return GSM_FT_ERR;
	}

	return GSM_OK;
}

GSM_ERR GSM_DeInit(gsm_t *gsm)
{
	if (gsm == NULL) return GSM_INVALID_ARGUMENT;

	gsm->ft = NULL;
	gsm->uart_rcvr_gsm = NULL;
	gsm->uart_rcvr_debug = NULL;

	FT_KillTimer(gsm->ft, gsm->timers.time_counter);
	FT_KillTimer(gsm->ft, gsm->timers.timeout);
	FT_KillTimer(gsm->ft, gsm->timers.detect);

	memset(gsm, 0, sizeof(gsm_t));

	return GSM_OK;
}


GSM_ERR GSM_ClearResponse(gsm_t *gsm)
{
	if(gsm == NULL) return GSM_INVALID_ARGUMENT;
	if(gsm->ft == NULL) return GSM_NOT_INITIALIZED;

	gsm->flags.timeout = 0;
	gsm->flags.response_expected = 0;
	gsm->flags.response_available = 0;
	gsm->flags.response_awaiting = 0;

	gsm->flags.response_ok = 0;
	gsm->flags.response_error = 0;
	gsm->flags.response_unknown = 0;
	gsm->flags.response_command = 0;
	gsm->flags.response_lines = 0;

	gsm->rx_buf_len = 0;
	gsm->rx_buf_processed = 0;
	gsm->line_buf_len = 0;

	return GSM_OK;
}

GSM_ERR GSM_GetLine(gsm_t *gsm)
{
	if(gsm == NULL) return GSM_INVALID_ARGUMENT;
	if(gsm->ft == NULL || gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

    if(gsm->rx_buf_len == 0 || gsm->rx_buf_len == gsm->rx_buf_processed) return GSM_LINE_NO_DATA;

    gsm->line_buf_len = 0;
	while (gsm->rx_buf_processed < gsm->rx_buf_len && gsm->line_buf_len < GSM_LINE_BUFFER_SIZE - 1) {
		if (gsm->rx_buf_processed && gsm->rx_buf[gsm->rx_buf_processed - 1] == '\r' && gsm->rx_buf[gsm->rx_buf_processed] == '\n')
		{
			gsm->rx_buf_processed++;
			gsm->line_buf_len--;
			break;
		}
		gsm->line_buf[gsm->line_buf_len++] = gsm->rx_buf[gsm->rx_buf_processed++];
	}
	gsm->line_buf[gsm->line_buf_len] = 0;

	return GSM_OK;
}
