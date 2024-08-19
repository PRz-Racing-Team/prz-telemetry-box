///*
// * frame_collector.c
// *
// *  Created on: Aug 18, 2024
// *      Author: Dariusz Strojny @ReijiY
// */
//
//#include "frame_collector.h"
//
//
//
//void FC_FeedFrame(frame_collector_t *fc, uint16_t frame_id, uint8_t frame_len, uint8_t *frame_data)
//{
//	if (fc == NULL || frame_data == NULL || frame_len == 0) return;
//
//	uint16_t object_id = FC_FrameObjectID(frame_id);
//	if (object_id == 0)
//	{
//		char buffer[64];
//		snprintf(buffer, sizeof(buffer), "Unknown frame ID: 0x%04X", frame_id);
//		prints(buffer);
//		return;
//	}
//
//	if (object_id != fc->last_object_id || fc->current_buffer == NULL) {
//		if (fc->current_buffer != NULL) {
//			prints("Dropping frame buffer\r\n");
//			FC_FreeBuffer(fc, fc->last_object_id);
//		}
//
//		uint16_t expected_frames_count = FC_GetExpectedFramesCount(object_id, fc->ams_config);
//
//		// Number of ICs, thermistors and cells is not known at this point, so we can't collect objects that depend on them
//		if (expected_frames_count == 0) return;
//
//
//		if (FC_MallocBuffer(fc, object_id) == 0) return;
//
//
//		fc->last_object_id = object_id;
//		fc->frames_count = 0;
//		fc->expected_frames_count = expected_frames_count;
//	}
//
//
//	if (frame_id == BMS_CAN_ID_ICS_CONFIG)
//	{
//		bms_values_ams_config *ams_config = (bms_values_ams_config*) fc->current_buffer;
//		ams_config->ics_count = frame_data[0];
//		ams_config->ics_cells_count = frame_data[1];
//		ams_config->ics_thermistors_count = frame_data[2];
//		uint32_t cells_slots = frame_data[3] | (frame_data[4] << 8) | (frame_data[5] << 16);
//		uint16_t thermistors_slots = frame_data[6] | (frame_data[7] << 8);
//		for (uint8_t i = 0; i < BMS_CELLS_MAX; i++)
//		{
//			ams_config->cells_slots[i] = (cells_slots >> i) & 1;
//		}
//		for (uint8_t i = 0; i < BMS_THERMISTORS_MAX; i++)
//		{
//			ams_config->thermistors_slots[i] = (thermistors_slots >> i) & 1;
//		}
//	}
////	else if (frame_id == BMS_CAN_ID_ADC_VALUES) {
////		bms_values_adc *adc = (bms_values_adc*) fc->current_buffer;
////		adc->adc_values[0] = (frame_data[0] << 8) | frame_data[1];
////		adc->adc_values[1] = (frame_data[2] << 8) | frame_data[3];
////	} else if (frame_id == BMS_CAN_ID_SOC) {
////		bms_values_adc *adc = (bms_values_adc*) fc->current_buffer;
////		adc->soc = frame_data[0];
////	} else if (frame_id == BMS_CAN_ID_IC_MIN_MAX_VOLTAGES) {
////		bms_values_voltages *voltages =
////				(bms_values_voltages*) fc->current_buffer;
////		voltages->ics_min_max_total_voltages[0] = (frame_data[0] << 8)
////				| frame_data[1];
////		voltages->ics_min_max_total_voltages[1] = (frame_data[2] << 8)
////				| frame_data[3];
////	} else if (frame_id == BMS_CAN_ID_IC_DELTA_VOLTAGES) {
////		bms_values_voltages *voltages =
////				(bms_values_voltages*) fc->current_buffer;
////		voltages->ics_delta_voltages[0] = (frame_data[0] << 8) | frame_data[1];
////		voltages->ics_delta_voltages[1] = (frame_data[2] << 8) | frame_data[3];
////	} else if (frame_id == BMS_CAN_ID_CELL_MIN_MAX_VOLTAGES) {
////		bms_values_voltages *voltages =
////				(bms_values_voltages*) fc->current_buffer;
////		voltages->cell_min_max_voltages[0] = (frame_data[0] << 8)
////				| frame_data[1];
////	}
//
//
//
//	fc->frames_count++;
//
//	if (fc->frames_count == fc->expected_frames_count)
//	{
//		if (object_id == CCB_ID_AMS_CONFIG) {
//			if (fc->ams_config == NULL)
//			{
//				fc->ams_config = (bms_values_ams_config*)malloc(sizeof(bms_values_ams_config));
//				if (fc->ams_config == NULL) {
//					prints("Failed to allocate memory for AMS config\r\n");
//					FC_FreeBuffer(fc, CCB_ID_AMS_CONFIG);
//					return;
//				}
//			}
//			memcpy(fc->ams_config, fc->current_buffer, sizeof(bms_values_ams_config));
//
//			char buffer[64];
//			snprintf(buffer, sizeof(buffer), "AMS Config: %d ICs, %d cells, %d thermistors\r\n", fc->ams_config->ics_count, fc->ams_config->ics_cells_count, fc->ams_config->ics_thermistors_count);
//			prints(buffer);
//		}
//
//
//
//		FC_FreeBuffer(fc, object_id);
//
//		fc->last_object_id = 0;
//		fc->frames_count = 0;
//
////		char buffer[64];
////		snprintf(buffer, sizeof(buffer), "Object %4X complete\r\n", object_id);
////		prints(buffer);
//	}
//
//}
//
//uint32_t FC_GetObject(frame_collector_t *fc, uint32_t buffer_size, char *buffer);
//
//uint16_t FC_FrameObjectID(uint16_t frame_id)
//{
//	if (frame_id == BMS_CAN_ID_ERROR)
//	{
//		return CCB_ID_AMS_ERROR;
//	}
//	else if (frame_id == BMS_CAN_ID_BALANCING_CELL_STATES ||
//			frame_id == BMS_CAN_ID_BALANCING_STATES)
//	{
//		return CCB_ID_BALANCING;
//	}
//	else if (frame_id == BMS_CAN_ID_TRANSOPTOR_STATES)
//	{
//		return CCB_ID_TRANSOPTORS;
//	}
//	else if (frame_id == BMS_CAN_ID_SAFETY_STATE)
//	{
//		return CCB_ID_SAFETY;
//	}
//	else if (frame_id == BMS_CAN_ID_THERM_VREFS ||
//			frame_id == BMS_CAN_ID_THERM_GPIO_VOLTAGES ||
//			frame_id == BMS_CAN_ID_THERM_TEMPERATURES ||
//			frame_id == BMS_CAN_ID_ACCUMULATOR_TEMPERATURE_MIN_MAX_AVG)
//	{
//		return CCB_ID_TEMPERATURES;
//	}
//	else if (frame_id == BMS_CAN_ID_CELL_VOLTAGES ||
//			frame_id == BMS_CAN_ID_CELL_DELTAS ||
//			frame_id == BMS_CAN_ID_IC_MIN_MAX_VOLTAGES ||
//			frame_id == BMS_CAN_ID_IC_DELTA_VOLTAGES ||
//			frame_id == BMS_CAN_ID_ACCUMULATOR_MIN_MAX_TOTAL_VOLTAGES ||
//			frame_id == BMS_CAN_ID_CELL_MIN_MAX_VOLTAGES)
//	{
//		return CCB_ID_VOLTAGES;
//	}
//	else if (frame_id == BMS_CAN_ID_IC_DETECTED_STATES)
//	{
//		return CCB_ID_ICS_CONNECTIONS;
//	}
//	else if (frame_id == BMS_CAN_ID_ICS_CONFIG)
//	{
//		return CCB_ID_AMS_CONFIG;
//	}
//	else if (frame_id == BMS_CAN_ID_ADC_VALUES ||
//			frame_id == BMS_CAN_ID_SOC)
//	{
//		return CCB_ID_ADC;
//	}
//
//	return 0;
//}
//
//uint16_t FC_GetExpectedFramesCount(uint16_t object_id, bms_values_ams_config *ams_config)
//{
//	// Number of ICs, thermistors and cells is not known at this point, so we can't collect objects that depend on them
//	if (object_id == 0 || (ams_config == NULL && (
//				object_id == CCB_ID_BALANCING ||
//				object_id == CCB_ID_TEMPERATURES ||
//				object_id == CCB_ID_VOLTAGES )))
//	{
//		return 0;
//	}
//
//	if (object_id == CCB_ID_AMS_ERROR)
//	{
//		return 1;
//	}
//	else if (object_id == CCB_ID_BALANCING)
//	{
//		return ams_config->ics_count + 1;
//	}
//	else if (object_id == CCB_ID_TRANSOPTORS)
//	{
//		return 1;
//	}
//	else if (object_id == CCB_ID_SAFETY)
//	{
//		return 1;
//	}
//	else if (object_id == CCB_ID_TEMPERATURES)
//	{
//		return FC_Ceil(ams_config->ics_count, 3) +
//				2 * ams_config->ics_count * FC_Ceil(ams_config->ics_thermistors_count, 3) +
//				1;
//	}
//	else if (object_id == CCB_ID_VOLTAGES)
//	{
//		return 2 * ams_config->ics_count * FC_Ceil(ams_config->ics_cells_count, 3) +
//				ams_config->ics_count +
//				FC_Ceil(ams_config->ics_count, 3) +
//				2;
//	}
//	else if (object_id == CCB_ID_ICS_CONNECTIONS)
//	{
//		return 1;
//	}
//	else if (object_id == CCB_ID_AMS_CONFIG)
//	{
//		return 1;
//	}
//	else if (object_id == CCB_ID_ADC)
//	{
//		return 2;
//	}
//
//	return 0;
//}
//
//uint8_t FC_Init(frame_collector_t *fc)
//{
//	if (fc == NULL) return 0;
//	FC_Free(fc);
//
//	uint8_t err = GCB_Init(&fc->text_buffer, FC_MAX_FRAMES);
//	if (err == 0) return 0;
//
//	return 1;
//}
//
//void FC_Free(frame_collector_t *fc)
//{
//	if (fc == NULL) return;
//
//	GCB_Free(&fc->text_buffer);
//
//	if (fc->current_buffer != NULL) free(fc->current_buffer);
//	fc->current_buffer = NULL;
//	if (fc->ams_config != NULL) free(fc->ams_config);
//	fc->ams_config = NULL;
//	fc->last_object_id = 0;
//	fc->frames_count = 0;
//}
//
//uint16_t FC_Ceil(uint16_t value, uint16_t divisor)
//{
//	if (divisor == 0) return 0;
//	return (value + divisor - 1) / divisor;
//}
//
//uint8_t FC_MallocBuffer(frame_collector_t *fc, uint16_t object_id)
//{
//	if (fc == NULL || object_id == 0) return 0;
//
//
//	switch (object_id)
//	{
//	case CCB_ID_AMS_ERROR:
//		fc->current_buffer = malloc(sizeof(bms_values_ams_error));
//		break;
//	case CCB_ID_BALANCING:
//		fc->current_buffer = malloc(sizeof(bms_values_balancing));
//		break;
//	case CCB_ID_TRANSOPTORS:
//		fc->current_buffer = malloc(sizeof(bms_values_transoptors));
//		break;
//	case CCB_ID_SAFETY:
//		fc->current_buffer = malloc(sizeof(bms_values_safety));
//		break;
//	case CCB_ID_TEMPERATURES:
//		fc->current_buffer = malloc(sizeof(bms_values_temperatures));
//		break;
//	case CCB_ID_VOLTAGES:
//		fc->current_buffer = malloc(sizeof(bms_values_voltages));
//		break;
//	case CCB_ID_ICS_CONNECTIONS:
//		fc->current_buffer = malloc(sizeof(bms_values_ic_connections));
//		break;
//	case CCB_ID_AMS_CONFIG:
//		fc->current_buffer = malloc(sizeof(bms_values_ams_config));
//		break;
//	case CCB_ID_ADC:
//		fc->current_buffer = malloc(sizeof(bms_values_adc));
//		break;
//	default:
//		prints("Unknown object ID\r\n");
//		return 0;
//	}
//
//	if (fc->current_buffer == NULL)
//	{
//		prints("Failed to allocate memory for frame buffer\r\n");
//		return 0;
//	}
//
//	switch (object_id)
//	{
//	case CCB_ID_AMS_ERROR:
//		memset(fc->current_buffer, 0, sizeof(bms_values_ams_error));
//		break;
//	case CCB_ID_BALANCING:
//		memset(fc->current_buffer, 0, sizeof(bms_values_balancing));
//		break;
//	case CCB_ID_TRANSOPTORS:
//		memset(fc->current_buffer, 0, sizeof(bms_values_transoptors));
//		break;
//	case CCB_ID_SAFETY:
//		memset(fc->current_buffer, 0, sizeof(bms_values_safety));
//		break;
//	case CCB_ID_TEMPERATURES:
//		memset(fc->current_buffer, 0, sizeof(bms_values_temperatures));
//		break;
//	case CCB_ID_VOLTAGES:
//		memset(fc->current_buffer, 0, sizeof(bms_values_voltages));
//		break;
//	case CCB_ID_ICS_CONNECTIONS:
//		memset(fc->current_buffer, 0, sizeof(bms_values_ic_connections));
//		break;
//	case CCB_ID_AMS_CONFIG:
//		memset(fc->current_buffer, 0, sizeof(bms_values_ams_config));
//		break;
//	case CCB_ID_ADC:
//		memset(fc->current_buffer, 0, sizeof(bms_values_adc));
//		break;
//	}
//
//	return 1;
//}
//
//void FC_FreeBuffer(frame_collector_t *fc, uint16_t object_id)
//{
//	if (fc == NULL || fc->current_buffer == NULL) return;
//	if (object_id == CCB_ID_AMS_ERROR) free((bms_values_ams_error*)fc->current_buffer);
//	else if (object_id == CCB_ID_BALANCING) free((bms_values_balancing*)fc->current_buffer);
//	else if (object_id == CCB_ID_TRANSOPTORS) free((bms_values_transoptors*) fc->current_buffer);
//	else if (object_id == CCB_ID_SAFETY) free((bms_values_safety*) fc->current_buffer);
//	else if (object_id == CCB_ID_TEMPERATURES) free((bms_values_temperatures*) fc->current_buffer);
//	else if (object_id == CCB_ID_VOLTAGES) free((bms_values_voltages*) fc->current_buffer);
//	else if (object_id == CCB_ID_ICS_CONNECTIONS) free((bms_values_ic_connections*) fc->current_buffer);
//	else if (object_id == CCB_ID_AMS_CONFIG) free((bms_values_ams_config*) fc->current_buffer);
//	else if (object_id == CCB_ID_ADC) free((bms_values_adc*) fc->current_buffer);
//	else
//	{
//		prints("Freeing unknown object ID\r\n");
//		free(fc->current_buffer);
//	}
//	fc->current_buffer = NULL;
//}
