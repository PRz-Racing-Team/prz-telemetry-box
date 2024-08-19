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
	if (cfcb == NULL || CFCB_IsFull(cfcb)) return 0;
	__disable_irq();
	can_frame_t *can_frame = (can_frame_t *)malloc(sizeof(can_frame_t));
	__enable_irq();
	if (can_frame == NULL) return 0;

	can_frame->id = id;
	can_frame->len = len;
	memcpy(can_frame->data, data, len);

	cfcb->elements[cfcb->head] = can_frame;
	cfcb->head = (cfcb->head + 1) % GCB_MAX_CAN_FRAMES;

	return 1;
}

uint8_t CFCB_Pop(can_frames_cb_t *cfcb, can_frame_t *can_frame)
{
	if (cfcb == NULL || CFCB_IsEmpty(cfcb) || can_frame == NULL) return 0;
	can_frame_t *frame = cfcb->elements[cfcb->tail];
    can_frame->id = frame->id;
    can_frame->len = frame->len;
    memcpy(can_frame->data, frame->data, frame->len);
    free(frame);
	cfcb->tail = (cfcb->tail + 1) % GCB_MAX_CAN_FRAMES;
	return 1;
}
