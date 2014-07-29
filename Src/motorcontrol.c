/*
 * motorcontrol.c
 *
 *  Created on: 2014.07.29.
 *      Author: G�bor
 */

#include "motorcontrol.h"

void MotorThread(void const * argument) {
	uint32_t previousWakeTime = osKernelSysTick();

	for(;;) {
		osDelayUntil(&previousWakeTime, 10);

	}
}
