/*
 * fancy_timer_defines_errors.c
 *
 *  Created on: May 26, 2024
 *      Author: Dariusz Strojny @ReijiY
 */

#include "fancy_timer_defines_errors.h"


const char * FT_ErrorToString(FT_ERR code) {
	switch (code)
	{
	default:
		return FT_INVALID_ERR_CODE_STR;

	case FT_OK:
		return FT_OK_STR;
	case FT_NOT_INITIALIZED:
		return FT_NOT_INITIALIZED_STR;
	case FT_OUT_OF_MEMORY :
		return FT_OUT_OF_MEMORY_STR ;
	case FT_INVALID_ARGUMENT:
		return FT_INVALID_ARGUMENT_STR;

	case FT_INVALID_ID:
		return FT_INVALID_ID_STR;
	case FT_INVALID_INDEX:
		return FT_INVALID_INDEX_STR;
	case FT_INVALID_TIM_INSTANCE:
		return FT_INVALID_TIM_INSTANCE_STR;

	case FT_IMPOSSIBLE_FREQUENCY:
		return FT_IMPOSSIBLE_FREQUENCY_STR;
	case FT_TRIGGERED:
		return FT_TRIGGERED_STR;
	case FT_ERRORED:
		return FT_ERRORED_STR;
	case FT_NO_TIMER:
		return FT_NO_TIMER_STR;

	}
	return FT_INVALID_ERR_CODE_STR;
}
