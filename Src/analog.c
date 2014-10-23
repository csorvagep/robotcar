/*
 * analog.c
 *
 *  Created on: 2014.08.17.
 *      Author: Gábor
 */

#include "analog.h"
#include "sensor.h"

#include <stdio.h>

void AnalogThread(void const * argument __attribute__((unused))) {
	uint32_t previousWakeTime = osKernelSysTick();
	uint32_t vbatm, vbate;

	for (;;) {
		osDelayUntil(&previousWakeTime, 1000);

		vbatm = BSP_Sensor_GetVBATM() * 9000 >> 12; //divide by 4096
		vbate = BSP_Sensor_GetVBATE() * 9000 >> 12; //divide by 4096
		//printf("B,%lu,%lu\r\n", vbate, vbatm);
	}
}
