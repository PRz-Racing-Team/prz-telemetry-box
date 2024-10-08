/*
 * gsm_defines_errors.h
 *
 *  Created on: Jul 23, 2024
 *      Author: Darisz Strojny @ReijiY
 */

#ifndef GSM_DEFINES_ERRORS__H_
#define GSM_DEFINES_ERRORS__H_


#define GSM_ERR_LIB_MASK					0xF000
#define GSM_ERR_CODE_MASK					0x0FFF

#define GSM_ERR 							uint16_t
#define GSM_ERR_BASE						0x9000
#define GSM_ERR_(X)							((GSM_ERR)X | GSM_ERR_BASE)
#define GSM_ERR_TEST(X)						((X & GSM_ERR_LIB_MASK) == GSM_ERR_BASE)
#define GSM_INVALID_ERR_CODE_STR			"GSM_INVALID_ERROR_CODE"

#define	GSM_OK								GSM_ERR_(0x00)
#define GSM_OK_STR							"GSM_OK"

#define GSM_UNIFY_OK(X)						(((X & GSM_ERR_CODE_MASK) == (GSM_OK & GSM_ERR_CODE_MASK)) ? GSM_OK : X)

#define	GSM_NOT_INITIALIZED					GSM_ERR_(0x01)
#define GSM_NOT_INITIALIZED_STR				"GSM_NOT_INITIALIZED"

#define	GSM_OUT_OF_MEMORY 					GSM_ERR_(0x02)
#define GSM_OUT_OF_MEMORY_STR 				"GSM_OUT_OF_MEMORY"

#define	GSM_INVALID_ARGUMENT				GSM_ERR_(0x03)
#define GSM_INVALID_ARGUMENT_STR			"GSM_INVALID_ARGUMENT"

#define GSM_IDLE                            GSM_ERR_(0x04)
#define GSM_IDLE_STR                        "GMS_IDLE"

#define GSM_BUSY                            GSM_ERR_(0x05)
#define GSM_BUSY_STR                        "GSM_BUSY"

#define GSM_TIMEOUT                         GSM_ERR_(0x06)
#define GSM_TIMEOUT_STR                     "GSM_TIMEOUT"

#define GSM_BAUD_RATE_ERR	            	GSM_ERR_(0x07)
#define GSM_BAUD_RATE_ERR_STR				"GSM_BAUD_RATE_ERR"



#define GSM_FT_ERR							GSM_ERR_(0x10)
#define GSM_FT_ERR_STR						"GSM_FT_ERR"


#define GSM_LINE_NO_DATA					GSM_ERR_(0x20)
#define GSM_LINE_NO_DATA_STR				"GSM_LINE_NO_DATA"


#define GSM_AT_ERR							GSM_ERR_(0x30)
#define GSM_AT_ERR_STR						"GSM_AT_ERR"


#define	GSM_HAL_ERR							GSM_ERR_(0x51)
#define GSM_HAL_ERR_STR						"GSM_HAL_ERR"

#define	GSM_HAL_BUSY						GSM_ERR_(0x52)
#define GSM_HAL_BUSY_STR					"GSM_HAL_BUSY"

#define	GSM_HAL_TIMEOUT						GSM_ERR_(0x53)
#define GSM_HAL_TIMEOUT_STR					"GSM_HAL_TIMEOUT"

#define	GSM_HAL_UNKNOWN						GSM_ERR_(0x54)
#define GSM_HAL_UNKNOWN_STR					"GSM_HAL_UNKNOWN"

#define GSM_ERROR                           GSM_ERR_(0xFF)
#define GSM_ERROR_STR                       "GSM_ERROR"


#endif /* GSM_DEFINES_ERRORS__H_ */
