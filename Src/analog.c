/*
 * analog.c
 *
 *  Created on: 2014.08.17.
 *      Author: G�bor
 */

#include "analog.h"
#include "sensor.h"

#include "communication.h"
#include <stdio.h>

void AnalogThread(void const * argument __attribute__((unused))) {
	uint32_t previousWakeTime = osKernelSysTick();
	uint32_t vbatm, vbate;
	char outputBuffer[16];

	for (;;) {
		osDelayUntil(&previousWakeTime, 1000);

		vbatm = BSP_Sensor_GetVBATM() * 9000 >> 12; //divide by 4096
		vbate = BSP_Sensor_GetVBATE() * 9000 >> 12; //divide by 4096
		sprintf(outputBuffer, "B,%lu,%lu\r\n", vbate, vbatm);
		SendString(outputBuffer);
	}
}
