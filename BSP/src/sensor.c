/*
 * sensor.c
 *
 *  Created on: 2014.07.29.
 *      Author: Gábor
 */

#include "sensor.h"

#define VBATM_OFFSET		0
#define VBATM_INDEX			1

#define VBATE_OFFSET		0
#define VBATE_INDEX			0

static uint32_t adcData[2];

void BSP_Sensor_Init(void) {
	//HAL_ADC_Start_DMA(&hadc1, adcData, 2);
}

uint16_t BSP_Sensor_GetVBATM(void) {
	return (uint32_t) adcData[VBATM_INDEX] + VBATM_OFFSET;
}
uint16_t BSP_Sensor_GetVBATE(void) {
	return (uint32_t) adcData[VBATE_INDEX] + VBATE_OFFSET;
}

