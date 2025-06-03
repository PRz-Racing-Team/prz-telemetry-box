/*
 * can_frames_cb.c
 *
 *  Created on: Aug 19, 2024
 *      Author: reiko
 */

#include "can_frames_cb.h"

uint8_t CFCB_Init(can_frames_cb_t *cfcb)
{
	if (cfcb == NULL) return 0;
	memset(cfcb->elements, 0, sizeof(cfcb->elements));
	cfcb->head = 0;
	cfcb->tail = 0;
	cfcb->counter = 0;
	return 1;
}

uint8_t CFCB_IsEmpty(can_frames_cb_t *cfcb)
{
	if (cfcb == NULL) return 1;
	return cfcb->head == cfcb->tail;
}

uint8_t CFCB_IsFull(can_frames_cb_t *cfcb)
{
	if (cfcb == NULL) return 1;
	return (cfcb->head + 1) % GCB_MAX_CAN_FRAMES == cfcb->tail;
}

uint16_t CFCB_GetSize(can_frames_cb_t *cfcb)
{
	if (cfcb == NULL) return 0;
	return (cfcb->head - cfcb->tail + GCB_MAX_CAN_FRAMES) % GCB_MAX_CAN_FRAMES;
}

uint8_t CFCB_Push(can_frames_cb_t *cfcb, uint16_t id, uint8_t len, uint8_t *data)
{
	__disable_irq();
	if (cfcb == NULL || CFCB_IsFull(cfcb)) {
		__enable_irq();
		return 0;
	}

	cfcb->elements[cfcb->head].id = id;
	cfcb->elements[cfcb->head].len = len;
	cfcb->elements[cfcb->head].counter = cfcb->counter;
	memcpy(cfcb->elements[cfcb->head].data, data, len);

	cfcb->head = (cfcb->head + 1) % GCB_MAX_CAN_FRAMES;

	__enable_irq();
	return 1;
}

uint8_t CFCB_Pop(can_frames_cb_t *cfcb, can_frame_t *can_frame)
{
	if (cfcb == NULL || CFCB_IsEmpty(cfcb) || can_frame == NULL) return 0;
    memcpy(can_frame, &cfcb->elements[cfcb->tail], sizeof(can_frame_t));
	cfcb->tail = (cfcb->tail + 1) % GCB_MAX_CAN_FRAMES;
	return 1;
}
