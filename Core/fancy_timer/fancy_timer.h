/*
 * fancy-timer.h
 *
 *  Created on: Feb 16, 2024
 *      Author: Dariusz Strojny @ReijiY
 */

#ifndef FANCY_TIMER_FANCY_TIMER_H_
#define FANCY_TIMER_FANCY_TIMER_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "fancy_timer_defines_errors.h"

#define FT_DEFAULT_SIZE 4 // should be a power of 2


#ifdef STM32H747xx
#include "stm32h7xx_hal.h"
#elif defined(STM32F429xx)
#include "stm32f4xx_hal.h"
#else
#error Unknown module!
#endif


typedef enum FT_TIMEBASE {
	FT_TB_UNKNOWN,
	FT_TB_US,
	FT_TB_MS,
	FT_TB_SEC
} ft_timebase;

typedef enum FT_TYPE {
	FT_TYPE_UNKNOWN,
	FT_TYPE_HR,
	FT_TYPE_GP,
	FT_TYPE_BASIC,
	FT_TYPE_LOW_POWER,
	FT_TYPE_ADVANCED
} ft_type;

typedef enum FT_RESOLUTION {
	FT_RES_UNKNOWN,
	FT_RES_8,
	FT_RES_16,
	FT_RES_32
} ft_resolution;

typedef enum FT_SOURCE {
	FT_SRC_UNKNOWN,
	FT_SRC_APB1,
	FT_SRC_APB2
} ft_source;

typedef enum FT_PRESCALER_RANGE {
	FT_PRE_UNKNOWN,
	FT_PRE_1_65536,
	FT_PRE_POW_2_DIV_1_4_MUL_2_32,
	FT_PRE_POW_2_MUL_1_128
} ft_prescaler_range;

typedef struct FT_CONFIG {
	ft_timebase 		timebase;

	uint32_t 			src_freq; // -- required for calculations
	uint32_t 			des_freq; // -- required for calculations
	float 				frequency;
	uint8_t 			is_stable;

	uint32_t 			prescaler;
	uint32_t 			max_prescaler; // -- required for calculations
	uint32_t 			divider;
	uint32_t 			counter_period; // -- required for calculations

	uint16_t 			cycle_length;
	uint32_t			cycle_freq;
	uint32_t			cycle_ms;
} ft_config;

// structure defining timer configuraion
typedef struct FT_DEF {
	uint16_t 			timer_id;
	uint16_t			priority;
	uint8_t 			paused;

	uint32_t 			trigger_counter;

	uint32_t 			requested_time;
	uint32_t 			elapsed_time;

	FT_ERR				last_error;
} ft_def;

typedef struct FT_BASE {
	TIM_HandleTypeDef 	*htimer;

	ft_type 			type;
	ft_source 			source;
	ft_resolution 		resolution;

	ft_config			config;

	ft_def** 			timers;
	uint16_t 			timers_size;
	uint16_t 			timers_count;

	uint32_t			ticks_remainder;
	uint32_t			SRCR; // software repetition counter :)

	double tick_freq;

	// Optional handlers
	UART_HandleTypeDef *huart;
} FT_base;

extern FT_base* ft;

FT_ERR FT_IT_Feed(FT_base* ft);

void FT_Print(FT_base* ft, const char* str);

FT_ERR FT_Init(FT_base** ft, UART_HandleTypeDef *huart, TIM_HandleTypeDef* htim, ft_timebase timebase);
FT_ERR FT_InitCustom(FT_base** ft, UART_HandleTypeDef *huart, TIM_HandleTypeDef* htim, uint32_t target_frequency);
void FT_Free(FT_base* ft);

FT_ERR FT_ProcessTimer(FT_base* ft, uint32_t feed_time, ft_def* timer);
FT_ERR FT_ProcessTimerByID(FT_base* ft, uint32_t feed_time, uint16_t id);
FT_ERR FT_ProcessTimerByIndex(FT_base* ft, uint32_t feed_time, uint16_t index);

FT_ERR FT_TriggerTimerN(FT_base* ft, uint16_t timer_id, uint16_t trigger_count);
FT_ERR FT_TriggerTimer(FT_base* ft, uint16_t timer_id);

FT_ERR FT_Delay(FT_base* ft, uint32_t time);
FT_ERR FT_DelayUntilTimer(FT_base* ft, uint16_t timer_id);

FT_ERR FT_Clear(FT_base* ft);
FT_ERR FT_Feed(FT_base* ft);

FT_ERR FT_GetTimerTriggerCount(FT_base* ft, uint16_t timer_id, uint16_t* trigger_count);
FT_ERR FT_GetTimerPauseState(FT_base* ft, uint16_t timer_id, uint8_t* state);

FT_ERR FT_GetTriggeredTimer(FT_base* ft, uint16_t* id, uint32_t* trigger_count);
FT_ERR FT_GetTriggeredTimerAny(FT_base* ft, uint16_t* id, uint16_t* trigger_count);

FT_ERR FT_GetErroredTimer(FT_base* ft, uint16_t* id, uint16_t* error_count);

FT_ERR FT_SetTimerInterval(FT_base* ft, uint16_t timer_id, uint32_t interval);
FT_ERR FT_SetTimerPriority(FT_base* ft, uint16_t timer_id, uint16_t priority);
FT_ERR FT_SetTimerTimeBase(FT_base* ft, uint16_t timer_id, uint32_t time);

FT_ERR FT_SetTimerPauseState(FT_base* ft, uint16_t timer_id, uint8_t state);
FT_ERR FT_PauseTimer(FT_base* ft, uint16_t timer_id);
FT_ERR FT_ResumeTimer(FT_base* ft, uint16_t timer_id);

FT_ERR FT_StartTimer(FT_base* ft, uint16_t timer_id);
FT_ERR FT_StopTimer(FT_base* ft, uint16_t timer_id);

FT_ERR FT_ResetTimer(FT_base* ft, uint16_t timer_id);
FT_ERR FT_ClearTimer(FT_base* ft, uint16_t timer_id);

FT_ERR FT_NewTimer(FT_base* ft, uint32_t time, uint16_t priority, uint16_t* timer_id);

FT_ERR FT_KillTimer(FT_base* ft, uint16_t id);

uint16_t FT_NewTimerID_(FT_base* ft);

FT_ERR FT_GetTimer_(FT_base* ft, uint16_t timer_id, ft_def** timer);

FT_ERR FT_PrintConfiguration(FT_base* ft, char* str, uint32_t max_length);

uint32_t FT_GetHTimerFrequency(TIM_HandleTypeDef* htim);
float FT_FindPrescaler(TIM_HandleTypeDef* htim, uint32_t freq);

uint32_t FT_GetTimebaseFrequency(ft_timebase timebase);

ft_type FT_GetType(TIM_HandleTypeDef* htim);
ft_source FT_GetSource(TIM_HandleTypeDef* htim);
ft_resolution FT_GetResolution(TIM_HandleTypeDef* htim);
ft_prescaler_range FT_GetPrescalerRange(TIM_HandleTypeDef* htim);
uint32_t FT_GetFrequency(ft_source source);

FT_ERR FT_GetTimerConfiguration(ft_config* config);

#endif /* FANCY_TIMER_FANCY_TIMER_H_ */
