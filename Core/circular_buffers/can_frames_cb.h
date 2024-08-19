/*
 * generic_circular_buffer.h
 *
 *  Created on: Aug 19, 2024
 *      Author: Dariusz Strojny @ReijiY
 */

#ifndef CAN_FRAMES_CB__H_
#define CAN_FRAMES_CB__H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <cmsis_gcc.h>

#define GCB_MAX_CAN_FRAMES 32

typedef struct {
	uint16_t id;
	uint8_t len;
	uint8_t data[8];
} can_frame_t;

typedef struct {
	can_frame_t* elements[GCB_MAX_CAN_FRAMES];
	uint16_t head;
	uint16_t tail;
} can_frames_cb_t;

uint8_t CFCB_Init(can_frames_cb_t *cfcb);

uint8_t CFCB_IsEmpty(can_frames_cb_t *cfcb);
uint8_t CFCB_IsFull(can_frames_cb_t *cfcb);
uint16_t CFCB_GetSize(can_frames_cb_t *cfcb);

uint8_t CFCB_Push(can_frames_cb_t *cfcb, uint16_t id, uint8_t len, uint8_t *data);
uint8_t CFCB_Pop(can_frames_cb_t *cfcb, can_frame_t *can_frame);


#endif /* CAN_FRAMES_CB__H_ */
