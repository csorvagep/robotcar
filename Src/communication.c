/*
 * communication.c
 *
 *  Created on: 2014.07.28.
 *      Author: G�bor
 */

#include "communication.h"

#include "bluetooth.h"
//#include "radio.h"
//#include <stdio.h>

static osMutexId sendMutex = NULL;

void CommThread(void const * argument __attribute__((unused))) {
	uint32_t previousWakeTime = osKernelSysTick();

	sendMutex = osMutexCreate(NULL);
	configASSERT(sendMutex);

	for (;;) {
		osDelayUntil(&previousWakeTime, 50);

		/* Empty output buffer */
		if (osMutexWait(sendMutex, 3) == osOK) {
			BSP_BT_Flush();
			osMutexRelease(sendMutex);
		}
		//printf("%d,%d\r\n", BSP_Radio_GetMotor(), BSP_Radio_GetSteer());
	}
}

void SendChars(const char * str, size_t len) {
	if (sendMutex != NULL) {
		if (osMutexWait(sendMutex, 3) == osOK) {
			BSP_BT_SendChars(str, len);
			osMutexRelease(sendMutex);
		}
	}
}
