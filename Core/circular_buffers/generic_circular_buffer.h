/*
 * generic_circular_buffer.h
 *
 *  Created on: Aug 18, 2024
 *      Author: Dariusz Strojny @ReijiY
 */

#ifndef GENERIC_CIRCULAR_BUFFER__H_
#define GENERIC_CIRCULAR_BUFFER__H_

#include <stdlib.h>
#include <stdint.h>

typedef struct {
	uint16_t buffer_size;
	void** elements;
	uint16_t head;
	uint16_t tail;
} generic_circular_buffer_t;

uint8_t GCB_Init(generic_circular_buffer_t *gcb, uint16_t buffer_size);
void GCB_Free(generic_circular_buffer_t *gcb);

uint8_t GCB_IsEmpty(generic_circular_buffer_t *gcb);
uint8_t GCB_IsFull(generic_circular_buffer_t *gcb);
uint16_t GCB_GetSize(generic_circular_buffer_t *gcb);

uint8_t GCB_Push(generic_circular_buffer_t *gcb, void *element);
void* GCB_Pop(generic_circular_buffer_t *gcb);


#endif /* GENERIC_CIRCULAR_BUFFER__H_ */
