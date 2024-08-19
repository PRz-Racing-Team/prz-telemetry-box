/*
 * generic_circular_buffer.c
 *
 *  Created on: Aug 18, 2024
 *      Author: Dariusz Strojny @ReijiY
 */

#include "../circular_buffers/generic_circular_buffer.h"


uint8_t GCB_Init(generic_circular_buffer_t *gcb, uint16_t buffer_size)
{
	if (gcb == NULL || buffer_size == 0) return 0;
	GCB_Free(gcb);

	gcb->buffer_size = buffer_size;
	gcb->elements = (void**)malloc(sizeof(void*) * buffer_size);
	if (gcb->elements == NULL) return 0;

	gcb->head = 0;
	gcb->tail = 0;

	return 1;
}

void GCB_Free(generic_circular_buffer_t *gcb)
{
	if (gcb == NULL) return;

	if(gcb->elements != NULL) free(gcb->elements);
	gcb->elements = NULL;
}

uint8_t GCB_IsEmpty(generic_circular_buffer_t *gcb)
{
	if(gcb == NULL) return 1;
	return gcb->head == gcb->tail;
}

uint8_t GCB_IsFull(generic_circular_buffer_t *gcb)
{
	if (gcb == NULL || gcb->buffer_size == 0) return 1;
	return (gcb->head + 1) % gcb->buffer_size == gcb->tail;
}

uint16_t GCB_GetSize(generic_circular_buffer_t *gcb)
{
	if (gcb == NULL || gcb->buffer_size == 0) return 0;
	return (gcb->head - gcb->tail + gcb->buffer_size) % gcb->buffer_size;
}

uint8_t GCB_Push(generic_circular_buffer_t *gcb, void *element)
{
	if (element == NULL || GCB_IsFull(gcb)) return 0;
	gcb->elements[gcb->head] = element;
	gcb->head = (gcb->head + 1) % gcb->buffer_size;
	return 1;
}

void* GCB_Pop(generic_circular_buffer_t *gcb)
{
	if (gcb == NULL || GCB_IsEmpty(gcb)) return NULL;
	void *element = gcb->elements[gcb->tail];
	gcb->tail = (gcb->tail + 1) % gcb->buffer_size;
	return element;
}
