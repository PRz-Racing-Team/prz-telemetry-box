/*
 * fancy-timer.c
 *
 *  Created on: Feb 16, 2024
 *      Author: ReijiY
 */

#include "../fancy_timer/fancy_timer.h"

FT_base* ft = NULL;

FT_ERR FT_IT_Feed(FT_base* ft)
{
	if(ft == NULL) return FT_INVALID_ARGUMENT;
	ft->SRCR++;
	return FT_OK;
}

void FT_Print(FT_base* ft, const char* str)
{
	if(ft != NULL && ft->huart != NULL)
		HAL_UART_Transmit(ft->huart, (uint8_t *)str, strlen(str), 100);
}

FT_ERR FT_Init(FT_base** ft, UART_HandleTypeDef *huart, TIM_HandleTypeDef* htim, ft_timebase timebase)
{
	uint32_t target_frequency = FT_GetTimebaseFrequency(timebase);
	if(target_frequency == 0) return FT_INVALID_ARGUMENT;
	return FT_InitCustom(ft, huart, htim, target_frequency);
}

FT_ERR FT_InitCustom(FT_base** ftimer, UART_HandleTypeDef *huart, TIM_HandleTypeDef* htim, uint32_t target_frequency)
{
	if(ftimer == NULL || htim == NULL || target_frequency == 0)
	{
		return FT_INVALID_ARGUMENT;
	}

	ft_timebase timebase = FT_TB_UNKNOWN;
	if(target_frequency == 1) timebase = FT_TB_SEC;
	if(target_frequency == 1000) timebase = FT_TB_MS;
	if(target_frequency == 1000000) timebase = FT_TB_US;

	ft_source source = FT_GetSource(htim);
	ft_type type = FT_GetType(htim);
	ft_resolution resolution = FT_GetResolution(htim);
	ft_prescaler_range prescaler_range = FT_GetPrescalerRange(htim);

	if(source == FT_SRC_UNKNOWN ||
		type == FT_TYPE_UNKNOWN ||
		resolution == FT_RES_UNKNOWN ||
		prescaler_range == FT_PRE_UNKNOWN )
	{
		return FT_INVALID_TIM_INSTANCE;
	}

	uint32_t src_freq = FT_GetFrequency(source);
	if(src_freq == 0)
	{
		return FT_INVALID_TIM_INSTANCE;
	}

	uint32_t max_prescaler;
	switch(prescaler_range)
	{
	case FT_PRE_1_65536: max_prescaler = 65536; break;
	default: return FT_INVALID_TIM_INSTANCE;
	}

	uint32_t counter_period;
	switch(resolution)
	{
	case FT_RES_8: counter_period = UINT8_MAX; break;
	case FT_RES_16: counter_period = UINT16_MAX; break;
	case FT_RES_32: counter_period = UINT32_MAX; break;
	default: return FT_INVALID_TIM_INSTANCE;
	}

	if(target_frequency > src_freq)
	{
		return FT_INVALID_ARGUMENT;
	}


	ft_config config;
	config.timebase = timebase;
	config.src_freq = src_freq;
	config.des_freq = target_frequency;
	config.max_prescaler = max_prescaler;
	config.counter_period = counter_period;

	FT_ERR res = FT_GetTimerConfiguration(&config);

	if(res != FT_OK) return res;

	if(*ftimer != NULL) {
		FT_Free(*ftimer);
		*ftimer = NULL;
	}

	FT_base* ft = (FT_base*)malloc(sizeof(FT_base));
	if(ft == NULL) return FT_OUT_OF_MEMORY;
	memset(ft, 0, sizeof(FT_base));


	ft->timers_count = 0;
	ft->timers_size = FT_DEFAULT_SIZE;
	ft->timers = (ft_def**)malloc(sizeof(ft_def*) * ft->timers_size);
	if(ft->timers == NULL) {
		free(ft);
		return FT_OUT_OF_MEMORY;
	}
	memset(ft->timers, 0, sizeof(ft_def*) * ft->timers_size);

	ft->source = source;
	ft->type = type;
	ft->resolution = resolution;

	// that's software now
	//ft->has_repetition_counter = IS_TIM_REPETITION_COUNTER_INSTANCE(htim->Instance);

	ft->ticks_remainder = 0;

	ft->config = config;

	ft->htimer = htim;
	ft->huart = huart;

	*ftimer = ft;

	// update timer configuration
	htim->Instance->PSC = config.prescaler;
	htim->Instance->ARR = config.counter_period;

	/*if(ft->has_repetition_counter)
	{
		// enable repetition counter (if available)
		htim->Instance->RCR = FT_REPETITION_COUNTER;
	}*/

	if(IS_TIM_REPETITION_COUNTER_INSTANCE(htim->Instance))
	{
		htim->Instance->RCR = 0;
	}

	// set flag to update prescaler and repetition counter
	htim->Instance->EGR = TIM_EGR_UG;

	// start timer
	HAL_TIM_Base_Start(htim);

	return FT_OK;
}

void FT_Free(FT_base* ft)
{
	if(ft == NULL) return;
	free(ft->timers);
	free(ft);
}


FT_ERR FT_ProcessTimer(FT_base* ft, uint32_t feed_time, ft_def* timer)
{
	if(ft == NULL || timer == NULL) return FT_INVALID_ARGUMENT;
	if(timer->paused) return FT_OK;

	uint8_t triggered = 0;
	//uint8_t errored = 0;

	//uint8_t dbgprnt = 0;
	if(feed_time > 30000)
	{
//		snprintf(print_buf, sizeof(print_buf), "1. feed_time: %lu, requested_time: %lu, trigger_counter: %lu, elapsed: %lu\r\n",
//						feed_time, timer->requested_time, timer->trigger_counter, timer->elapsed_time);
//		FT_Print(ft, print_buf);
		//dbgprnt = 1;
	}

	// Long delay
	// ~> Timer triggered in one cycle
	if(feed_time >= timer->requested_time) {
		uint32_t tgc = feed_time / timer->requested_time;
		feed_time -= tgc * timer->requested_time;
		timer->trigger_counter += tgc;
		triggered = 1;
	}

//
//	if(dbgprnt) {
//		snprintf(print_buf, sizeof(print_buf), "2. feed_time: %lu, requested_time: %lu, trigger_counter: %lu, elapsed: %lu\r\n",
//						feed_time, timer->requested_time, timer->trigger_counter, timer->elapsed_time);
//		FT_Print(ft, print_buf);
//	}

	// Overflow
	// ~> At least UINT32_MAX elapsed since timer started
	// ~> At least one requested_time got fulfilled
	if(timer->elapsed_time + feed_time < timer->elapsed_time)
	{
		timer->elapsed_time += feed_time - timer->requested_time;
		timer->trigger_counter++;
		triggered = 1;


//		if(dbgprnt) {
//			snprintf(print_buf, sizeof(print_buf), "OF feed_time: %lu, requested_time: %lu, trigger_counter: %lu, elapsed: %lu\r\n",
//							feed_time, timer->requested_time, timer->trigger_counter, timer->elapsed_time);
//			FT_Print(ft, print_buf);
//		}
	}
	else
	{
		timer->elapsed_time += feed_time;

		// Regular trigger
		while(timer->elapsed_time >= timer->requested_time)
		{
			timer->elapsed_time -= timer->requested_time;
			timer->trigger_counter++;
			triggered = 1;
		}


//		if(dbgprnt) {
//			snprintf(print_buf, sizeof(print_buf), "END feed_time: %lu, requested_time: %lu, trigger_counter: %lu, elapsed: %lu\r\n",
//							feed_time, timer->requested_time, timer->trigger_counter, timer->elapsed_time);
//			FT_Print(ft, print_buf);
//		}
	}


//	if(dbgprnt) {
//		snprintf(print_buf, sizeof(print_buf), "feed_time: %lu, requested_time: %lu, trigger_counter: %lu, elapsed: %lu\r\n",
//						feed_time, timer->requested_time, timer->trigger_counter, timer->elapsed_time);
//		FT_Print(ft, print_buf);
//	}

	if(triggered) return FT_TRIGGERED;
	//else if(errored) return FT_ERRORED;
	return FT_OK;
}

FT_ERR FT_ProcessTimerByID(FT_base* ft, uint32_t feed_time, uint16_t id)
{
	if(ft == NULL) return FT_INVALID_ARGUMENT;
	for(uint16_t i = 0; i < ft->timers_count; i++)
	{
		if(ft->timers[i]->timer_id == id) return FT_ProcessTimer(ft, feed_time, ft->timers[i]);
	}
	return FT_INVALID_ID;
}

FT_ERR FT_ProcessTimerByIndex(FT_base* ft, uint32_t feed_time, uint16_t index)
{
	if(ft == NULL) return FT_INVALID_ARGUMENT;
	if(index >= ft->timers_size || ft->timers[index] == NULL) return FT_INVALID_INDEX;

	return FT_ProcessTimer(ft, feed_time, ft->timers[index]);
}

FT_ERR FT_TriggerTimerN(FT_base* ft, uint16_t timer_id, uint16_t trigger_count)
{
	if (ft == NULL) return FT_INVALID_ARGUMENT;
	ft_def* timer = NULL;
	if (FT_GetTimer_(ft, timer_id, &timer) == FT_OK)
	{
		timer->trigger_counter += trigger_count;
		return FT_OK;
	}
	return FT_INVALID_ID;
}

FT_ERR FT_TriggerTimer(FT_base* ft, uint16_t timer_id)
{
	return FT_TriggerTimerN(ft, timer_id, 1);
}

FT_ERR FT_Delay(FT_base* ft, uint32_t time)
{
	if(ft == NULL) return FT_INVALID_ARGUMENT;
	FT_Feed(ft);
	while(ft->htimer->Instance->CNT < time) {}
	return FT_OK;
}

FT_ERR FT_DelayUntilTimer(FT_base* ft, uint16_t timer_id)
{
	if(ft == NULL) return FT_INVALID_ARGUMENT;
	ft_def* timer = NULL;
	FT_ERR res = FT_GetTimer_(ft, timer_id, &timer);
	if(res != FT_OK) return res;

	while(timer->trigger_counter == 0)
	{
		FT_Feed(ft);
	}

	return FT_OK;
}

FT_ERR FT_Clear(FT_base* ft)
{
	if(ft == NULL) return FT_INVALID_ARGUMENT;

	for(uint16_t i = 0; i < ft->timers_count; i++)
	{
		if(ft->timers[i] != NULL) {
			ft->timers[i]->elapsed_time = 0;
			ft->timers[i]->trigger_counter = 0;
			ft->timers[i]->last_error = FT_OK;
		}
	}

	ft->ticks_remainder = 0;
	ft->htimer->Instance->CNT = 0;
	ft->SRCR = 0;
	return FT_OK;
}

FT_ERR FT_Feed(FT_base* ft)
{
	if(ft == NULL) return FT_INVALID_ARGUMENT;

	// recalculate existing timers

	uint8_t timers_triggered = 0;
	uint8_t timers_errored = 0;
	FT_ERR res;


	uint32_t feed_time = ft->htimer->Instance->CNT; //__HAL_TIM_GET_COUNTER(ft->htimer)

	if(feed_time) ft->htimer->Instance->CNT = 0;


	if(ft->config.divider > 1)
	{
		feed_time += ft->ticks_remainder;

		ft->ticks_remainder = feed_time % ft->config.divider;

		feed_time /= ft->config.divider;
	}

	if(ft->SRCR)
	{
		feed_time += ft->SRCR * ft->config.counter_period;

		ft->SRCR = 0;
	}

	if(feed_time == 0) return FT_OK;


	for(uint16_t i = 0; i < ft->timers_count; i++)
	{
		if(ft->timers[i] != NULL)
		{
			res = FT_ProcessTimer(ft, feed_time, ft->timers[i]);

			if(res == FT_ERRORED)
			{
				timers_errored++;
			}
			else
			{
				if(ft->timers[i]->trigger_counter) timers_triggered++;
				else if(ft->timers[i]->last_error != FT_OK) timers_errored++;
			}
		}
	}

	if(timers_triggered) return FT_TRIGGERED;
	if(timers_errored) return FT_ERRORED;
	return FT_OK;
}

FT_ERR FT_GetTimerTriggerCount(FT_base* ft, uint16_t timer_id, uint16_t* trigger_count)
{
	if(ft == NULL || trigger_count == NULL) return FT_INVALID_ARGUMENT;
	ft_def* timer = NULL;

	FT_ERR res = FT_GetTimer_(ft, timer_id, &timer);
	if(res == FT_OK)
	{
		*trigger_count = timer->trigger_counter;
	}
	return res;
}

FT_ERR FT_GetTimerPauseState(FT_base* ft, uint16_t timer_id, uint8_t* state)
{
	if (ft == NULL || state == NULL) return FT_INVALID_ARGUMENT;
	ft_def *timer = NULL;
	FT_ERR res = FT_GetTimer_(ft, timer_id, &timer);
	if (res == FT_OK) {
		*state = timer->paused;
	}
	return res;
}

FT_ERR FT_GetTriggeredTimer(FT_base* ft, uint16_t* id, uint32_t* trigger_count)
{
	if(ft == NULL || id == NULL) return FT_INVALID_ARGUMENT;

	uint32_t timer_i = UINT32_MAX;
	uint16_t priority = 0;

	for(uint16_t i = 0; i < ft->timers_count; i++)
	{
		if(ft->timers[i] != NULL && ft->timers[i]->trigger_counter && ft->timers[i]->priority > priority)
		{
			timer_i = i;
			priority = ft->timers[i]->priority;
		}
	}

	if(timer_i != UINT32_MAX) {
		*id = ft->timers[timer_i]->timer_id;
		if(trigger_count != NULL) *trigger_count = ft->timers[timer_i]->trigger_counter;
		ft->timers[timer_i]->trigger_counter = 0;
		return FT_OK;
	}
	return FT_NO_TIMER;
}

FT_ERR FT_GetTriggeredTimerAny(FT_base* ft, uint16_t* id, uint16_t* trigger_count)
{
	if(ft == NULL || id == NULL) return FT_INVALID_ARGUMENT;

	for(uint16_t i = 0; i < ft->timers_count; i++)
	{
		if(ft->timers[i] != NULL && ft->timers[i]->trigger_counter)
		{
			*id = ft->timers[i]->timer_id;
			if(trigger_count != NULL) *trigger_count = ft->timers[i]->trigger_counter;
			ft->timers[i]->trigger_counter = 0;
			return FT_TRIGGERED;
		}
	}
	return FT_OK;
}

FT_ERR FT_GetErroredTimer(FT_base* ft, uint16_t* id, uint16_t* last_error)
{
	if(ft == NULL) return FT_INVALID_ARGUMENT;

	for(uint16_t i = 0; i < ft->timers_count; i++)
	{
		if(ft->timers[i] != NULL && ft->timers[i]->last_error != FT_OK)
		{
			if(last_error != NULL) *last_error = ft->timers[i]->last_error;
			ft->timers[i]->last_error = FT_OK;
			if(id != NULL) *id = ft->timers[i]->timer_id;
			return FT_OK;
		}
	}
	return FT_NO_TIMER;
}

FT_ERR FT_SetTimerInterval(FT_base* ft, uint16_t timer_id, uint32_t interval)
{
	if (ft == NULL) return FT_INVALID_ARGUMENT;
	ft_def *timer = NULL;
	FT_ERR res = FT_GetTimer_(ft, timer_id, &timer);
	if (res == FT_OK) {
		timer->requested_time = interval;
		return FT_OK;
	}
	return res;
}

FT_ERR FT_SetTimerPriority(FT_base* ft, uint16_t timer_id, uint16_t priority)
{
	if(ft == NULL) return FT_INVALID_ARGUMENT;
	ft_def *timer = NULL;
	FT_ERR res = FT_GetTimer_(ft, timer_id, &timer);
	if(res == FT_OK) {
		timer->priority = priority;
		return FT_OK;
	}
	return res;
}

FT_ERR FT_SetTimerTimeBase(FT_base* ft, uint16_t timer_id, uint32_t time)
{
	if (ft == NULL) return FT_INVALID_ARGUMENT;
	ft_def *timer = NULL;
	FT_ERR res = FT_GetTimer_(ft, timer_id, &timer);
	if (res == FT_OK) {
		timer->requested_time = time;
		return FT_OK;
	}
	return res;
}

FT_ERR FT_ResetTimer(FT_base* ft, uint16_t timer_id)
{
	if (ft == NULL) return FT_INVALID_ARGUMENT;
	ft_def *timer = NULL;
	FT_ERR res = FT_GetTimer_(ft, timer_id, &timer);
	if (res == FT_OK) {
		timer->elapsed_time = 0;
		timer->trigger_counter = 0;
		timer->last_error = FT_OK;
		return FT_OK;
	}
	return res;
}

FT_ERR FT_ClearTimer(FT_base* ft, uint16_t timer_id)
{
	FT_ERR err = FT_PauseTimer(ft, timer_id);
	if(err == FT_OK) err = FT_ResetTimer(ft, timer_id);
	return err;
}

FT_ERR FT_SetTimerPauseState(FT_base* ft, uint16_t timer_id, uint8_t state)
{
	if(ft == NULL) return FT_INVALID_ARGUMENT;
	ft_def* timer = NULL;
	FT_ERR res = FT_GetTimer_(ft, timer_id, &timer);
	if (res == FT_OK) {
		timer->paused = state;
		return FT_OK;
	}
	return res;
}

FT_ERR FT_PauseTimer(FT_base* ft, uint16_t timer_id)
{
	return FT_SetTimerPauseState(ft, timer_id, 1);
}

FT_ERR FT_ResumeTimer(FT_base* ft, uint16_t timer_id)
{
	return FT_SetTimerPauseState(ft, timer_id, 0);
}

FT_ERR FT_StartTimer(FT_base* ft, uint16_t timer_id)
{
	if(ft == NULL) return FT_INVALID_ARGUMENT;
	ft_def* timer = NULL;
	FT_ERR res = FT_GetTimer_(ft, timer_id, &timer);
	if (res == FT_OK) {
		timer->elapsed_time = 0;
		timer->trigger_counter = 0;
		timer->last_error = FT_OK;
		timer->paused = 0;
		return FT_OK;
	}
	return res;
}

FT_ERR FT_StopTimer(FT_base* ft, uint16_t timer_id)
{
	if (ft == NULL) return FT_INVALID_ARGUMENT;
	ft_def *timer = NULL;
	FT_ERR res = FT_GetTimer_(ft, timer_id, &timer);
	if (res == FT_OK) {
		timer->elapsed_time = 0;
		timer->trigger_counter = 0;
		timer->last_error = FT_OK;
		timer->paused = 1;
		return FT_OK;
	}
	return res;
}

FT_ERR FT_NewTimer(FT_base* ft, uint32_t time, uint16_t priority, uint16_t* timer_id)
{
	if(ft == NULL || ft->timers_count == UINT16_MAX - 1) return FT_INVALID_ARGUMENT;


	if(ft->timers_count == ft->timers_size)
	{
		if(ft->timers_size == UINT16_MAX) return FT_OUT_OF_MEMORY;

		ft->timers_size *= 2;
		ft->timers = (ft_def**)realloc(ft->timers, sizeof(ft_def*) * ft->timers_size);
		if(ft->timers == NULL) return 0;
	}

	ft_def* new_timer = (ft_def*)malloc(sizeof(ft_def));
	if(new_timer == NULL) return FT_OUT_OF_MEMORY;

	memset(new_timer, 0, sizeof(ft_def));

	new_timer->timer_id = FT_NewTimerID_(ft);
	new_timer->requested_time = time;
	new_timer->last_error = FT_OK;
	new_timer->priority = priority;

	if(timer_id != NULL) *timer_id = new_timer->timer_id;

	ft->timers[ft->timers_count++] = new_timer;
	return FT_OK;
}

FT_ERR FT_KillTimer(FT_base* ft, uint16_t id)
{
	if(ft == NULL) return FT_INVALID_ARGUMENT;
	for(uint16_t i = 0; i < ft->timers_count; i++)
	{
		if(ft->timers[i]->timer_id == id) {
			free(ft->timers[i]);
			ft->timers_count--;
			if(ft->timers_count && i < ft->timers_count)
			{
				// Swap empty index with last timer
				ft->timers[i] = ft->timers[ft->timers_count];
				ft->timers[ft->timers_count] = NULL;
			}
			return FT_OK;
		}
	}
	return FT_INVALID_ID;
}

uint16_t FT_NewTimerID_(FT_base* ft)
{
	if(ft == NULL || ft->timers_count == UINT16_MAX - 1) return 0;

	// get highest id and increment it
	uint16_t max_id = 0;
	uint16_t timer_i;
	for(timer_i = 0; timer_i < ft->timers_count; timer_i++)
	{
		if(ft->timers[timer_i] == NULL) continue;
		if(ft->timers[timer_i]->timer_id >= max_id) {
			max_id = ft->timers[timer_i]->timer_id;
		}
	}
	if(max_id < UINT16_MAX) return max_id + 1;

	// if ran out of new ids, try to reuse old one
	uint16_t new_id = 1;
	uint16_t timers_compared = 0;
	timer_i = 0;
	while(timers_compared < ft->timers_count)
	{
		if(ft->timers[timer_i] == NULL) continue;
		if(ft->timers[timer_i]->timer_id == new_id)
		{
			new_id++;
			if(new_id == UINT16_MAX) break;
			timer_i = 0;
		}
		else timer_i++;
	}
	if(new_id < UINT16_MAX) return new_id;
	return 0;
}

FT_ERR FT_GetTimer_(FT_base* ft, uint16_t timer_id, ft_def** timer)
{
	if(ft == NULL || timer == NULL) return FT_INVALID_ARGUMENT;

	for(uint16_t i = 0; i < ft->timers_count; i++)
	{
		if(ft->timers[i]->timer_id == timer_id)
		{
			*timer = ft->timers[i];
			return FT_OK;
		}
	}
	return FT_INVALID_ID;
}

FT_ERR FT_PrintConfiguration(FT_base* ft, char* str, uint32_t max_length)
{
	if(ft == NULL || str == NULL || max_length == 0) return FT_INVALID_ARGUMENT;
	str[0] = '\0';
	snprintf(str, max_length, "srcfq:%lu\r\n"
						"fq/desfq:%f/%lu\r\n"
						"psc:%lu/%lu\r\n"
						"arr:%lu/%lu\r\n"
						"cycl/fq/ms:%u/%lu/%lu\r\n"
						"stable:%c\r\n",
			ft->config.src_freq,
			ft->config.frequency, ft->config.des_freq,
			ft->config.prescaler + 1, ft->config.max_prescaler,
			ft->config.divider, ft->config.counter_period,
			ft->config.cycle_length, ft->config.cycle_freq, ft->config.cycle_ms,
			ft->config.is_stable ? 'T' : 'F'
			);
	return FT_OK;
}

//void FT_Delay_us(TIM_HandleTypeDef* htim)
//{
//	__HAL_TIM_SET_COUNTER(htim_us,0);  // set the counter value a 0
//	while (__HAL_TIM_GET_COUNTER(htim_us) < us);  // wait for the counter to reach the us input in the parameter
//}

uint32_t FT_GetHTimerFrequency(TIM_HandleTypeDef* htim)
{
	if(htim == NULL) return 0;
	ft_source src = FT_GetSource(htim);
	if(src == FT_SRC_UNKNOWN) return 0;
	return FT_GetFrequency(src) / (htim->Init.Prescaler + 1);
}

float FT_FindPrescaler(TIM_HandleTypeDef* htim, uint32_t freq)
{
	if(htim == NULL || freq == 0) return -1;
	ft_source src = FT_GetSource(htim);
	if(src == FT_SRC_UNKNOWN) return -1;
	return ((float)FT_GetFrequency(src) / freq) - 1;
}

uint32_t FT_GetTimebaseFrequency(ft_timebase timebase)
{
	switch(timebase)
	{
	case FT_TB_US:
		return 1000000;
	case FT_TB_MS:
		return 1000;
	case FT_TB_SEC:
		return 1;
	default:
	}

	return 0;
}

ft_type FT_GetType(TIM_HandleTypeDef* htim)
{
	if (
#ifdef TIM1
			htim->Instance == TIM1 ||
#endif
#ifdef TIM8
			htim->Instance == TIM8 ||
#endif
			0)
	{
		return FT_TYPE_ADVANCED;
	}
	else if (
#ifdef TIM2
			htim->Instance == TIM2 ||
#endif
#ifdef TIM3
			htim->Instance == TIM3 ||
#endif
#ifdef TIM4
			htim->Instance == TIM4 ||
#endif
#ifdef TIM5
			htim->Instance == TIM5 ||
#endif
#ifdef TIM9
			htim->Instance == TIM9 ||
#endif
#ifdef TIM10
			htim->Instance == TIM10 ||
#endif
#ifdef TIM11
			htim->Instance == TIM11 ||
#endif
#ifdef TIM12
			htim->Instance == TIM12 ||
#endif
#ifdef TIM13
			htim->Instance == TIM13 ||
#endif
#ifdef TIM14
			htim->Instance == TIM14 ||
#endif
#ifdef TIM15
			htim->Instance == TIM15 ||
#endif
#ifdef TIM16
		    htim->Instance == TIM16 ||
#endif
#ifdef TIM17
			htim->Instance == TIM17 ||
#endif
			0)
	{
		return FT_TYPE_GP;
	}
	else if (
#ifdef TIM6
			htim->Instance == TIM6 ||
#endif
#ifdef TIM7
			htim->Instance == TIM7 ||
#endif
			0)
	{
		return FT_TYPE_BASIC;
	}
	return FT_TYPE_UNKNOWN;
}

ft_source FT_GetSource(TIM_HandleTypeDef* htim)
{
	if(
#ifdef TIM2
			htim->Instance == TIM2 ||
#endif
#ifdef TIM3
			htim->Instance == TIM3 ||
#endif
#ifdef TIM4
			htim->Instance == TIM4 ||
#endif
#ifdef TIM5
			htim->Instance == TIM5 ||
#endif
#ifdef TIM6
			htim->Instance == TIM6 ||
#endif
#ifdef TIM7
			htim->Instance == TIM7 ||
#endif
#ifdef TIM12
			htim->Instance == TIM12 ||
#endif
#ifdef TIM13
			htim->Instance == TIM13 ||
#endif
#ifdef TIM14
			htim->Instance == TIM14 ||
#endif
			0)
	{
		return FT_SRC_APB1;
	}
	else if(
#ifdef TIM1
			htim->Instance == TIM1 ||
#endif
#ifdef TIM8
			htim->Instance == TIM8 ||
#endif
#ifdef TIM9
			htim->Instance == TIM9 ||
#endif
#ifdef TIM10
			htim->Instance == TIM10 ||
#endif
#ifdef TIM11
			htim->Instance == TIM11 ||
#endif
#ifdef TIM15
			htim->Instance == TIM15 ||
#endif
#ifdef TIM16
			htim->Instance == TIM16 ||
#endif
#ifdef TIM17
			htim->Instance == TIM17 ||
#endif
		0)
	{
		return FT_SRC_APB2;
	}
	return FT_SRC_UNKNOWN;
}

ft_resolution FT_GetResolution(TIM_HandleTypeDef* htim)
{
	if(
#ifdef TIM1
		htim->Instance == TIM1 ||
#endif
#ifdef TIM3
		htim->Instance == TIM3 ||
#endif
#ifdef TIM4
		htim->Instance == TIM4 ||
#endif
#ifdef TIM6
		htim->Instance == TIM6 ||
#endif
#ifdef TIM7
		htim->Instance == TIM7 ||
#endif
#ifdef TIM8
		htim->Instance == TIM8 ||
#endif
#ifdef TIM9
		htim->Instance == TIM9 ||
#endif
#ifdef TIM10
		htim->Instance == TIM10 ||
#endif
#ifdef TIM11
		htim->Instance == TIM11 ||
#endif
#ifdef TIM12
		htim->Instance == TIM12 ||
#endif
#ifdef TIM13
		htim->Instance == TIM13 ||
#endif
#ifdef TIM14
		htim->Instance == TIM14 ||
#endif
#ifdef TIM15
		htim->Instance == TIM15 ||
#endif
#ifdef TIM16
		htim->Instance == TIM16 ||
#endif
#ifdef TIM17
		htim->Instance == TIM17 ||
#endif
		0)
	{
		return FT_RES_16;
	}
	else if(
#ifdef TIM2
		htim->Instance == TIM2 ||
#endif
#ifdef TIM5
		htim->Instance == TIM5 ||
#endif
		0)
	{
		return FT_RES_32;
	}
	return FT_RES_UNKNOWN;
}

ft_prescaler_range FT_GetPrescalerRange(TIM_HandleTypeDef* htim)
{
#if defined(STM32H747xx) || defined(STM32F429xx)
	return FT_PRE_1_65536;
#else
#error No prescaler values specified for current module!
#endif
}

uint32_t FT_GetFrequency(ft_source source)
{
	if (source == FT_SRC_APB1) {
		uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
#if defined(STM32H747xx)
		if((RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) == 0)
#else
		if((RCC->CFGR & RCC_CFGR_PPRE1) == 0)
#endif
		{
			/* PCLK1 prescaler equal to 1 => TIMCLK = PCLK1 */
			return (pclk1);
		}
		else
		{
			/* PCLK1 prescaler different from 1 => TIMCLK = 2 * PCLK1 */
			return(2 * pclk1);
		}
	}
	else if (source == FT_SRC_APB2) {
		uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();
#if defined(STM32H747xx)
		if((RCC->D2CFGR & RCC_D2CFGR_D2PPRE2) == 0)
#else
		if((RCC->CFGR & RCC_CFGR_PPRE2) == 0)
#endif
		{
			/* PCLK2 prescaler equal to 1 => TIMCLK = PCLK2 */
			return (pclk2);
		}
		else
		{
			/* PCLK2 prescaler different from 1 => TIMCLK = 2 * PCLK2 */
			return(2 * pclk2);
		}
	}
	return 0;
}

FT_ERR FT_GetTimerConfiguration(ft_config* config) {
	if(config == NULL) return FT_INVALID_ARGUMENT;


	uint32_t src_freq = config->src_freq;
	uint32_t des_freq = config->des_freq;
	uint32_t prescaler = 1;
	uint32_t max_prescaler = config->max_prescaler;
	uint32_t divider = 1;
	uint32_t counter_period = config->counter_period;


	if(max_prescaler == UINT32_MAX /*||
			counter_period == UINT32_MAX*/) return FT_INVALID_ARGUMENT;

    float prescaler_divider = (float)src_freq / des_freq;

    if(des_freq > src_freq || prescaler_divider / counter_period >= max_prescaler ) {
        return FT_IMPOSSIBLE_FREQUENCY;
    }


    float current_freq, current_freq_diff, best_freq_diff = src_freq;
    uint32_t best_prescaler = 1, best_divider = 1;

    for (divider = 1; divider <= counter_period; divider++)
    {
        for(prescaler = 1; prescaler <= max_prescaler; prescaler++)
        {
            current_freq = (float)src_freq / (prescaler * divider);
            current_freq_diff = fabs(des_freq - current_freq);
            if(current_freq == des_freq)
            {
                printf("Perfect hit!\r\n");

                config->is_stable = 1;

                config->prescaler = prescaler - 1;
                config->divider = divider;
                config->counter_period = (int)(counter_period / divider) * divider;

                config->frequency = (float)src_freq / (prescaler * divider);
                config->cycle_length = counter_period / divider;
                config->cycle_freq = ((float)src_freq / config->frequency) * config->cycle_length;
                config->cycle_ms = ((float)src_freq / config->frequency) * ((float)config->cycle_length / 1000);

                return FT_OK;
            }
            else if(current_freq_diff < best_freq_diff)
            {
                best_freq_diff = current_freq_diff;
                best_prescaler = prescaler;
                best_divider = divider;
            }
            else if(current_freq < des_freq)
            {
                break;
            }
        }
    }

    config->is_stable = 0;

    config->prescaler = best_prescaler - 1;
    config->divider = best_divider;
    config->counter_period = (int)(counter_period / best_divider) * best_divider;

    config->frequency = (float)src_freq / (best_prescaler * best_divider);
    config->cycle_length = counter_period / best_divider;
    config->cycle_freq = ((float)src_freq / config->frequency) * config->cycle_length;
    config->cycle_ms = config->cycle_freq / 1000;

    return FT_OK;
}
