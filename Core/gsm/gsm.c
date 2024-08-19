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

GSM_ERR GSM_PrintsData(gsm_t *gsm, const uint8_t *data, uint16_t len)
{
	if (gsm == NULL || data == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->uart_rcvr_debug == NULL) return GSM_OK;

	HAL_StatusTypeDef status = UartRcvr_send(gsm->uart_rcvr_debug, data, len);
	return status == HAL_OK ? GSM_OK : GSM_HAL_ERR;
}

GSM_ERR GSM_Prints(gsm_t *gsm, const char* str)
{
	if (gsm == NULL || str == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->uart_rcvr_debug == NULL) return GSM_OK;

	HAL_StatusTypeDef status = UartRcvr_print(gsm->uart_rcvr_debug, str);
	return status == HAL_OK ? GSM_OK : GSM_HAL_ERR;
}

GSM_ERR GSM_atData(gsm_t *gsm, const uint8_t* at_cmd, uint16_t len, uint8_t response_expected, uint32_t timeout_ms)
{
	if (gsm == NULL || at_cmd == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->ft == NULL || gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

	//GSM_Prints(gsm, ">> ");
	//GSM_PrintsData(gsm, at_cmd, len);

	GSM_ClearResponse(gsm);

	if (response_expected == 1)
	{
		gsm->flags.response.expected = 1;
		FT_SetTimerInterval(gsm->ft, gsm->timers.timeout, timeout_ms * 10);
		FT_StartTimer(gsm->ft, gsm->timers.timeout);
	}

	GSM_ERR err = GSM_cmd(gsm, at_cmd, len);
	if (err != GSM_OK) return err;

	return GSM_OK;
}

GSM_ERR GSM_at(gsm_t *gsm, const char* at_cmd, uint8_t response_expected, uint32_t timeout_ms)
{
	return GSM_atData(gsm, (const uint8_t*)at_cmd, strlen(at_cmd), response_expected, timeout_ms);
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
//		if(gsm->timer_id == gsm->timers.time_counter)
//		{
//			gsm->time += gsm->timer_trigger_count;
//		}
		if (gsm->timer_id == gsm->timers.timeout)
		{
			GSM_Prints(gsm, "Timeout\r\n");
			gsm->flags.response.timeout = 1;
			gsm->flags.timeout_count++;

			FT_StopTimer(gsm->ft, gsm->timers.timeout);

			if(gsm->flags.timeout_count >= 5)
			{
				gsm->flags.detected = 0;
				gsm->flags.initialized = 0;
				FT_StartTimer(gsm->ft, gsm->timers.detect);
			}

		}
		else if (gsm->timer_id == gsm->timers.detect)
		{
			if(gsm->flags.response.awaiting == 1) continue;
//			GSM_Prints(gsm, "Detect\r\n");
			if (gsm->flags.detected == 0)
			{
				if(gsm->flags.detecting && gsm->flags.response.timeout == 0 && gsm->flags.response.available == 0) continue;

				if(gsm->flags.timeout_count >= 5)
				{
					GSM_Prints(gsm, "Attempting to flood GSM\r\n");
					for(uint8_t i = 0; i < 200; i++)
					{
						GSM_at(gsm, "AT\r\n", 0, 1000);
					}
				}

				gsm->flags.detecting = 1;
				snprintf((char*)gsm->line_buf, GSM_LINE_BUFFER_SIZE, "GSM Dtct [%d]\r\n", gsm->flags.timeout_count + 1);
				GSM_Prints(gsm, (char*)gsm->line_buf);
				GSM_ChangeBaudRate(gsm);
				GSM_at(gsm, "\r\nAT\r\n", 1, 1000);
			}
			else {
				gsm->flags.detecting = 0;
				FT_StopTimer(gsm->ft, gsm->timers.detect);
			}
		}
		else
		{
			GSM_Prints(gsm, "Unknown timer triggered\r\n");
		}
	}

	if(gsm->flags.response.awaiting == 1) return GSM_BUSY;

	// module is detected
	if(gsm->flags.detected == 0)
	{
		uint8_t detecting_paused;
		if(FT_GetTimerPauseState(ft, gsm->timers.detect, &detecting_paused) != FT_OK) return GSM_FT_ERR;
		if(detecting_paused) FT_StartTimer(gsm->ft, gsm->timers.detect);
		return GSM_BUSY;
	}

	// module is initialized
	if (gsm->flags.initialized == 0)
	{
		if(gsm->flags.initializing == 1) return GSM_BUSY;
		return GSM_InitModem(gsm);
	}

	// network connection is opened
	if (gsm->flags.network_opened == 0) {
		if(gsm->flags.network_opening == 1) return GSM_BUSY;
		return GSM_OpenNetwork(gsm);
	}

	// UDP connection is opened
	if (gsm->flags.connection_opened == 0) {
		if (gsm->flags.connection_opening == 1) return GSM_BUSY;
		return GSM_OpenConnection(gsm);
	}

	return GSM_IDLE;
}

GSM_ERR GSM_AwaitResponse(gsm_t *gsm)
{
	if (gsm == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->ft == NULL || gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

	gsm->flags.response.awaiting = 1;

	while (gsm->flags.response.expected == 1 && gsm->flags.response.available == 0 && gsm->flags.response.timeout == 0)
	{
		GSM_Feed(gsm);
	}

	gsm->flags.response.awaiting = 0;
	gsm->flags.response.expected = 0;

	if (gsm->flags.response.timeout)
	{
		return GSM_TIMEOUT;
	}

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

		for (uint16_t i = 0; i < gsm->rx_buf_len; i++) {
			snprintf((char*)gsm->line_buf, GSM_LINE_BUFFER_SIZE, "%02x ", gsm->rx_buf[i]);
			GSM_Prints(gsm, (char*)gsm->line_buf);
		}

		while (GSM_GetLine(gsm) == GSM_OK)
		{
			if(gsm->flags.line_complete == 0) continue;
			gsm->flags.response.lines++;
			if(gsm->line_buf_len == 0) continue;

			GSM_Prints(gsm, "<< ");
			GSM_Prints(gsm, (char*)gsm->line_buf);
			GSM_Prints(gsm, "\r\n");



			if (gsm->line_buf_len >= 2 && strncmp((char*)gsm->line_buf, "OK", 2) == 0)
			{
				gsm->flags.response.has_ok = 1;
				gsm->flags.detected = 1;
				gsm->flags.response.available = 1;
			}
			else if (gsm->line_buf_len >= 5 && strncmp((char*)gsm->line_buf, "ERROR", 5) == 0)
			{
				gsm->flags.response.has_error = 1;
				gsm->flags.response.available = 1;
			}
			else if (strncmp((char*)gsm->line_buf, "+", 1) == 0)
			{
				if (gsm->line_buf_len >= 5 && strncmp((char*) gsm->line_buf, "+CMT", 4) == 0)
				{
					if(gsm->line_buf_len < 10 || gsm->line_buf[8] != ',') continue;
					int32_t pdu_len = atoi((char*)gsm->line_buf + 9);
					if(pdu_len == 0) continue;
					gsm->flags.sms_pdu_len = pdu_len;
					gsm->flags.receiving_sms = 1;
					continue;
				}
				GSM_ProcessCommand(gsm, (char*)gsm->line_buf, gsm->line_buf_len);
				gsm->flags.response.has_command = 1;
				gsm->flags.response.available = 1;
			}
			else if(gsm->line_buf_len >= 1 && strncmp((char*)gsm->line_buf, ">", 1) == 0)
			{
				gsm->flags.response.has_input_request = 1;
				gsm->flags.response.available = 1;
			}
			else if(gsm->flags.receiving_sms)
			{
				gsm->flags.receiving_sms = 0;
				GSM_DecodePDU(gsm, (char*)gsm->line_buf, gsm->line_buf_len);
				GSM_Prints(gsm, "SMS Received [not implemented]\r\n");
			}
			else {
				gsm->flags.response.has_unknown = 1;
			}

		}
		if(gsm->flags.line_complete == 0)
		{
			uint8_t retries = 50;
			while(!UartRcvr_available(gsm->uart_rcvr_gsm) && retries--)
			{
				HAL_Delay(1);
			}
		}
	}

	if (gsm->flags.response.available == 1) {
		gsm->flags.timeout_count = 0;
		gsm->flags.response.timeout = 0;
		FT_StopTimer(gsm->ft, gsm->timers.timeout);
	}

	return GSM_OK;
}

GSM_ERR GSM_InitModem(gsm_t *gsm)
{
	if (gsm == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->ft == NULL || gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

	gsm->flags.initializing = 1;

	HAL_Delay(100);

	GSM_Prints(gsm, "Initializing GSM Modem\r\n");

	GSM_at(gsm, "ATE0\r\n", 1, 1000); // Echo off
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response.has_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+IPR=921600\r\n", 1, 1000); // Set baud rate to 921600
	GSM_SetBaudRate(gsm, GSM_BAUD_RATE_921600);
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response.has_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CSCS=\"GSM\"\r\n", 1, 1000); // Set GSM character set
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response.has_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CMGF=0\r\n", 1, 1000); // sellect SMS PDU mode
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response.has_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CNMI=2,2,0,0,0\r\n", 1, 1000); // SMS notification
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response.has_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CUSD=1\r\n", 1, 1000); // USSD response
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response.has_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CGDCONT=1,\"IP\",\"internet\",\"0.0.0.0\",0,0\r\n", 1, 1000); // Set APN
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response.has_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CGACT=1,1\r\n", 1, 1000); // Activate PDP context
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response.has_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CIPRXGET=0\r\n", 1, 1000); // Enable automatic data receiving
	GSM_AwaitResponse(gsm);
	if (gsm->flags.response.has_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	// Unnecessary - we are using UDP connections only
	GSM_at(gsm, "AT+CIPCCFG=3,0,0,0,1,0,500\r\n", 1, 1000); // Set TCP connection mode
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response.has_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_at(gsm, "AT+CNTP=\"pool.ntp.org\",8\r\n", 1, 1000); // Set NTP server
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response.has_ok == 0) {
		gsm->flags.initializing = 0;
		return GSM_AT_ERR;
	}

	GSM_Prints(gsm, "GSM Modem initialized\r\n");

	gsm->flags.initializing = 0;
	gsm->flags.initialized = 1;

	return GSM_OK;
}

GSM_ERR GSM_OpenNetwork(gsm_t *gsm)
{
	if (gsm == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->ft == NULL || gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

	gsm->flags.network_opening = 1;

	GSM_Prints(gsm, "Opening network\r\n");

	//GSM_at(gsm, "AT+NETCLOSE\r\n", 1, 1000); // Close network

	GSM_at(gsm, "AT+NETOPEN\r\n", 1, 1000); // Open network
	GSM_AwaitResponse(gsm);
	if (gsm->flags.response.has_ok
			|| (gsm->flags.response.has_command
					&& GSM_CommandCompareName(gsm, "IP ERROR") == GSM_OK
					&& ( GSM_CommandCompareParameter(gsm, 0, "Network is already opened") == GSM_OK
							|| GSM_CommandCompareParameter(gsm, 0, "4"))))
	{
		gsm->flags.network_opening = 0;
		gsm->flags.network_opened = 1;

		if(gsm->flags.response.has_ok) GSM_Prints(gsm, "Network opened\r\n");
		else GSM_Prints(gsm, "Network was already opened\r\n");
		return GSM_OK;
	}

	gsm->flags.network_opening = 0;
	return GSM_AT_ERR;
}

GSM_ERR GSM_OpenConnection(gsm_t *gsm)
{
	if (gsm == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->ft == NULL || gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

	gsm->flags.connection_opening = 1;

	GSM_Prints(gsm, "Opening connection\r\n");

	GSM_at(gsm, "AT+CIPOPEN=0,\"UDP\",,,3333\r\n", 1, 1000); // Open UDP connection
	GSM_AwaitResponse(gsm);
	if (gsm->flags.response.has_command && GSM_CommandCompareName(gsm, "CIPOPEN") == GSM_OK
					&& (GSM_CommandCompareParameter(gsm, 1, "0") == GSM_OK || GSM_CommandCompareParameter(gsm, 1, "4") == GSM_OK))
	{
		gsm->flags.connection_opening = 0;
		gsm->flags.connection_opened = 1;

		if (GSM_CommandCompareParameter(gsm, 1, "0") == GSM_OK) GSM_Prints(gsm, "Connection opened\r\n");
		else GSM_Prints(gsm, "Connection was already opened\r\n");
		return GSM_OK;
	}

	gsm->flags.connection_opening = 0;
	return GSM_AT_ERR;
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

	FT_ERR ft_err = FT_OK;

	ft_err = FT_NewTimer(ft, GSM_TIMER_TIMEOUT_INTERVAL, GSM_TIMER_TIMEOUT_PRIORITY, &gsm->timers.timeout);
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

	FT_KillTimer(gsm->ft, gsm->timers.timeout);
	FT_KillTimer(gsm->ft, gsm->timers.detect);

	memset(gsm, 0, sizeof(gsm_t));

	return GSM_OK;
}


GSM_ERR GSM_ClearResponse(gsm_t *gsm)
{
	if(gsm == NULL) return GSM_INVALID_ARGUMENT;
	if(gsm->ft == NULL) return GSM_NOT_INITIALIZED;

	memset(&gsm->flags.response, 0, sizeof(gsm->flags.response));

	gsm->rx_buf_len = 0;
	gsm->rx_buf_processed = 0;
	gsm->line_buf_len = 0;
	gsm->flags.line_complete = 1;

	return GSM_OK;
}

GSM_ERR GSM_GetLine(gsm_t *gsm)
{
	if(gsm == NULL) return GSM_INVALID_ARGUMENT;
	if(gsm->ft == NULL || gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;

    if(gsm->rx_buf_len == 0 || gsm->rx_buf_len == gsm->rx_buf_processed) return GSM_LINE_NO_DATA;

    if(gsm->flags.line_complete)
    {
        gsm->line_buf_len = 0;

    }
	while (gsm->rx_buf_processed < gsm->rx_buf_len && gsm->line_buf_len < GSM_LINE_BUFFER_SIZE - 1) {
		if(gsm->rx_buf[gsm->rx_buf_processed] == '>')
		{
			gsm->line_buf[gsm->line_buf_len++] = gsm->rx_buf[gsm->rx_buf_processed++];
			gsm->flags.line_complete = 1;
			break;
		}
		if (gsm->rx_buf_processed && gsm->rx_buf[gsm->rx_buf_processed - 1] == '\r' && gsm->rx_buf[gsm->rx_buf_processed] == '\n')
		{
			gsm->rx_buf_processed++;
			gsm->line_buf_len--;
			gsm->flags.line_complete = 1;
			break;
		}
		gsm->line_buf[gsm->line_buf_len++] = gsm->rx_buf[gsm->rx_buf_processed++];
	}
	gsm->line_buf[gsm->line_buf_len] = 0;

	if(gsm->line_buf_len >= GSM_LINE_BUFFER_SIZE - 1) gsm->flags.line_complete = 1;

	return GSM_OK;
}

GSM_ERR GSM_DecodePDU(gsm_t *gsm, const char* pdu, uint16_t pdu_len)
{
	if(gsm == NULL || pdu == NULL) return GSM_INVALID_ARGUMENT;
	if(gsm->line_buf_len == 0) return GSM_INVALID_ARGUMENT;

	// TODO: implement PDU decoding

	memcpy(gsm->sms_buf, pdu, pdu_len);

	return GSM_OK;
}

GSM_ERR GSM_ProcessCommand(gsm_t *gsm, const char* cmd, uint16_t cmd_len)
{
	if (gsm == NULL || cmd == NULL || cmd_len == 0 || cmd[0] != '+') return GSM_INVALID_ARGUMENT;
	if (gsm->flags.response.has_command) return GSM_BUSY;

	if(cmd_len > GSM_COMMAND_BUFFER_SIZE)
	{
		cmd_len = GSM_COMMAND_BUFFER_SIZE;
		GSM_Prints(gsm, "Command buffer overflow. Truncating command\r\n");
	}

	char* pos = memchr(cmd, ':', cmd_len);
	if(pos == NULL || pos < cmd + 2) return GSM_INVALID_ARGUMENT;
	uint16_t cmd_name_len = pos - cmd - 1;

	gsm->flags.response.command_buf_name_len = cmd_name_len;

	memcpy(gsm->flags.response.command_buf, cmd, cmd_len);
	gsm->flags.response.command_buf_len = cmd_len;

	uint16_t i = cmd_name_len + 3;
	uint16_t param_len = 0;
	uint16_t params_count = 0;
	uint8_t inside_quotes = 0;

	while(i <= cmd_len)
	{
		if((cmd[i] == ',' || i == cmd_len) && inside_quotes == 0)
		{
			gsm->flags.response.command_buf_parameters_pos[params_count] = i - param_len;
			gsm->flags.response.command_buf_parameters_len[params_count] = param_len;
			params_count++;
			if(params_count >= GSM_COMMAND_MAX_PARAMETERS)
			{
				GSM_Prints(gsm, "Too many parameters in command. Truncating count\r\n");
				break;
			}
			param_len = 0;
		}
		else param_len++;
		if(cmd[i] == '"') inside_quotes = !inside_quotes;
		i++;
	}
	gsm->flags.response.command_buf_parameters_count = params_count;
	gsm->flags.response.has_command = 1;

	return GSM_OK;
}

GSM_ERR GSM_CommandCompareName(gsm_t *gsm, const char* str)
{
	if (gsm == NULL || str == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->flags.response.has_command == 0) return GSM_INVALID_ARGUMENT;

	if (gsm->flags.response.command_buf_name_len != strlen(str)) return GSM_ERROR;

	if (strncmp((char*)gsm->flags.response.command_buf + 1, str, gsm->flags.response.command_buf_name_len) == 0) return GSM_OK;
	return GSM_ERROR;
}

GSM_ERR GSM_CommandCompareParameter(gsm_t *gsm, uint16_t param_i, const char* str)
{
	if (gsm == NULL || str == NULL) return GSM_INVALID_ARGUMENT;
	size_t str_len = strlen(str);
	if (gsm->flags.response.has_command == 0 || str_len == 0) return GSM_INVALID_ARGUMENT;
	if (param_i >= gsm->flags.response.command_buf_parameters_count) return GSM_INVALID_ARGUMENT;

	if (gsm->flags.response.command_buf_parameters_len[param_i] != str_len) return GSM_ERROR;

	if(strncmp((char*)gsm->flags.response.command_buf + gsm->flags.response.command_buf_parameters_pos[param_i], str, str_len) == 0) return GSM_OK;

	return GSM_ERROR;
}

GSM_ERR GSM_CommandGetParameter(gsm_t *gsm, uint16_t param_i, char* buf, uint16_t buf_len, uint16_t* param_len)
{
	if (gsm == NULL || buf == NULL || param_len == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->flags.response.has_command == 0) return GSM_INVALID_ARGUMENT;
	if (param_i >= gsm->flags.response.command_buf_parameters_count) return GSM_INVALID_ARGUMENT;

	*param_len = buf_len < gsm->flags.response.command_buf_parameters_len[param_i] ? buf_len : gsm->flags.response.command_buf_parameters_len[param_i];

	memcpy(buf, (char*)gsm->flags.response.command_buf + gsm->flags.response.command_buf_parameters_pos[param_i], buf_len);

	return GSM_OK;
}

GSM_ERR GSM_CommandGetParameterInt(gsm_t *gsm, uint16_t param_i, int32_t* result)
{
	if (gsm == NULL || result == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->flags.response.has_command == 0) return GSM_INVALID_ARGUMENT;
	if (param_i >= gsm->flags.response.command_buf_parameters_count) return GSM_INVALID_ARGUMENT;

	*result = atoi((char*)gsm->flags.response.command_buf + gsm->flags.response.command_buf_parameters_pos[param_i]);

	return GSM_OK;
}

GSM_ERR GSM_CommandGetParameterFloat(gsm_t *gsm, uint16_t param_i, float* result)
{
	if (gsm == NULL || result == NULL) return GSM_INVALID_ARGUMENT;
	if (gsm->flags.response.has_command == 0) return GSM_INVALID_ARGUMENT;
	if (param_i >= gsm->flags.response.command_buf_parameters_count) return GSM_INVALID_ARGUMENT;

	*result = atof((char*)gsm->flags.response.command_buf + gsm->flags.response.command_buf_parameters_pos[param_i]);

	return GSM_OK;
}

GSM_ERR GSM_SendUDP(gsm_t *gsm, const uint8_t* data, uint16_t len)
{
	if (gsm == NULL || data == NULL || len == 0) return GSM_INVALID_ARGUMENT;
	if (gsm->uart_rcvr_gsm == NULL) return GSM_NOT_INITIALIZED;
	if (gsm->flags.data_sending == 1) return GSM_BUSY;

	gsm->flags.data_sending = 1;

	snprintf((char*)gsm->line_buf, GSM_LINE_BUFFER_SIZE, "AT+CIPSEND=0,%u,\"62.93.47.98\",3333\r\n", len);
	GSM_at(gsm, (char*)gsm->line_buf, 1, 1000);
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response.has_input_request == 0)
	{
		gsm->flags.data_sending = 0;
		return GSM_AT_ERR;
	}

	GSM_atData(gsm, data, len, 1, 3000);
	GSM_AwaitResponse(gsm);
	if(gsm->flags.response.has_ok == 0)
	{
		gsm->flags.data_sending = 0;
		return GSM_AT_ERR;
	}

	gsm->flags.data_sending = 0;

	return GSM_OK;
}

GSM_ERR GSM_SendUDPString(gsm_t *gsm, const char* str)
{
	return GSM_SendUDP(gsm, (const uint8_t*)str, strlen(str));
}




