/*
 * line_collector.h
 *
 *  Created on: Jul 17, 2024
 *      Author: Dariusz Strojny @ReijiY
 */

#ifndef LINE_COLLECTOR__H_
#define LINE_COLLECTOR__H_

#define MAX_LINE_LENGTH		256
#define MAX_LINES			4

#include <stdint.h>

typedef struct {
	char* line;
	uint16_t length;
} lc_line_t;

typedef struct {
	lc_line_t* lines;
	uint16_t count;

	uint8_t (*available)(void);
	uint16_t (*get_data)(char* buf, uint16_t buf_size);
} lc_t;

uint16_t LnClctr_available(lc_t *lc);
uint16_t LnClctr_get_line(lc_t *lc, char* buf, uint16_t buf_size);

void LnClctr_init(lc_t *lc, uint8_t (*available)(void), uint16_t (*get_data)(char* buf, uint16_t buf_size));




#endif /* LINE_COLLECTOR__H_ */
