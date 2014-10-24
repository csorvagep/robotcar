/*
 * deadreckoning.c
 *
 *  Created on: 2014 okt. 24
 *      Author: Gábor
 */

#include "deadreckoning.h"

#include <stdio.h>
#include "cmsis_os.h"
#include "stm32f4xx.h"
#include "stm32f4_discovery_accelerometer.h"
#include "communication.h"

void DeadReckoningThread(void const * argument) {
	uint32_t previousWakeTime;
	int16_t accelero[3];
	int16_t acceleroOffset[3];
	char outputBuffer[32];

	osDelay(30);

	BSP_ACCELERO_GetXYZ(acceleroOffset);

	previousWakeTime = osKernelSysTick();
	for (;;) {
		osDelayUntil(&previousWakeTime, 100);
		BSP_ACCELERO_GetXYZ(accelero);

		for (int i = 0; i < 3; i++)
			accelero[i] -= acceleroOffset[i];

		sprintf(outputBuffer, "G,%d,%d,%d\r\n", accelero[0], accelero[1],
				accelero[2]);
		SendString(outputBuffer);
	}
}
