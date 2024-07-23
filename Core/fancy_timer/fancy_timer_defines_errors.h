/*
 * fancy_timer_defines_errors.h
 *
 *  Created on: May 20, 2024
 *      Author: Dariusz Strojny/@ReijiY
 */

#ifndef FANCY_TIMER_DEFINES_ERRORS_H_
#define FANCY_TIMER_DEFINES_ERRORS_H_

#include <stdint.h>



#define FT_ERR 							uint16_t
#define FT_ERR_BASE						0x1000
#define FT_ERR_(X)						((FT_ERR)X | FT_ERR_BASE)
#define FT_ERR_TEST(X)					((X & 0xF000) == FT_ERR_BASE)
#define FT_INVALID_ERR_CODE_STR			"FT_INVALID_ERR_CODE"

#define FT_OK 							FT_ERR_(0x00)
#define FT_OK_STR 						"FT_OK"

#define FT_NOT_INITIALIZED 				FT_ERR_(0x01)
#define FT_NOT_INITIALIZED_STR 			"FT_NOT_INITIALIZED"

#define	FT_OUT_OF_MEMORY 				FT_ERR_(0x02)
#define FT_OUT_OF_MEMORY_STR 			"DC_OUT_OF_MEMORY"

#define FT_INVALID_ARGUMENT				FT_ERR_(0x03)
#define FT_INVALID_ARGUMENT_STR			"FT_INVALID_ARGUMENT"

#define FT_INVALID_ID					FT_ERR_(0x5)
#define FT_INVALID_ID_STR				"FT_INVALID_ID"

#define FT_INVALID_INDEX				FT_ERR_(0x6)
#define FT_INVALID_INDEX_STR			"FT_INVALID_INDEX"

#define FT_INVALID_TIM_INSTANCE			FT_ERR_(0x10)
#define FT_INVALID_TIM_INSTANCE_STR		"FT_INVALID_TIM_INSTANCE"

#define FT_IMPOSSIBLE_FREQUENCY			FT_ERR_(0x20)
#define FT_IMPOSSIBLE_FREQUENCY_STR		"FT_IMPOSSIBLE_FREQUENCY"

#define FT_TRIGGERED					FT_ERR_(0x50)
#define FT_TRIGGERED_STR				"FT_TRIGGERED"

#define FT_ERRORED						FT_ERR_(0x51)
#define FT_ERRORED_STR					"FT_ERRORED"

#define FT_NO_TIMER						FT_ERR_(0x52)
#define FT_NO_TIMER_STR					"FT_NO_TIMER"


const char * FT_ErrorToString(FT_ERR code);

#endif /* FANCY_TIMER_DEFINES_ERRORS_H_ */
