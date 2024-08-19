///*
// * frame_collector.h
// *
// *  Created on: Aug 18, 2024
// *      Author: Dariusz Strojny @ReijiY
// */
//
//#ifndef FRAME_COLLECTOR__H_
//#define FRAME_COLLECTOR__H_
//
//#include <stdint.h>
//
//// Imported from BMS project - path might need to be adjusted on different machines
//#include "../../../prz-bms/Common/bms/bms_common_typedefs.h"
//#include "../../../prz-bms/Common/bms/bms_defines.h"
//#include "../../../prz-bms/Common/cores_queue/core_circular_buffer_defines.h"
//
//#include "main.h"
//#include "../circular_buffers/generic_circular_buffer.h"
//
//#define FC_MAX_FRAMES 256
//
//typedef struct {
//	uint32_t length;
//	char *data;
//} fc_text_t;
//
//typedef struct {
//	fc_text_t text_buffers;
//
//	generic_circular_buffer_t text_buffer;
//	void *current_buffer;
//
//	uint16_t last_object_id;
//	uint16_t frames_count;
//	uint16_t expected_frames_count;
//
//	bms_values_ams_config *ams_config;
//} frame_collector_t;
//
//
//void FC_FeedFrame(frame_collector_t *fc, uint16_t frame_id, uint8_t frame_len, uint8_t *frame_data);
//uint32_t FC_GetObject(frame_collector_t *fc, uint32_t buffer_size, char *buffer);
//
//uint16_t FC_FrameObjectID(uint16_t frame_id);
//uint16_t FC_GetExpectedFramesCount(uint16_t object_id, bms_values_ams_config *ams_config);
//
//
//uint8_t FC_Init(frame_collector_t *fc);
//void FC_Free(frame_collector_t *fc);
//
//uint16_t FC_Ceil(uint16_t value, uint16_t divisor);
//
//uint8_t FC_MallocBuffer(frame_collector_t *fc, uint16_t object_id);
//void FC_FreeBuffer(frame_collector_t *fc, uint16_t object_id);
//
//
//
//
//
//
//#endif /* FRAME_COLLECTOR__H_ */
