/*
 * analog.c
 *
 *  Created on: 2014.08.17.
 *      Author: Gábor
 */

#include "analog.h"
#include "sensor.h"
#include "linesensor.h"

#include "communication.h"
#include <stdio.h>
#include <string.h>

#define SENSOR_DIST_MM		7.508f

static volatile FunctionalState printBatteryState = DISABLE;
static volatile FunctionalState printLineState = DISABLE;

static float linePos = 0.0f;
osSemaphoreId posMutex = NULL;
static uint8_t blackLimit = 100;

char sensorValToHexDigit(uint8_t val);

void AnalogThread(void const * argument __attribute__((unused))) {
	uint32_t previousWakeTime = osKernelSysTick();
	osSemaphoreId lineReadReadySemaphore = NULL;

	uint8_t sensorData[24] = { 0 };
	uint8_t blackCount = 0;
	uint32_t blackSum = 0;

	lineReadReadySemaphore = osSemaphoreCreate(NULL, 1);
	posMutex = osMutexCreate(NULL);
	uint8_t printCount = 0;

//	uint32_t vbatm, vbate;
	char outputBuffer[30];
//	uint8_t i = 0;

	for (;;) {
		osDelayUntil(&previousWakeTime, 10);

		BSP_Line_StartMeasure(lineReadReadySemaphore);
		osSemaphoreWait(lineReadReadySemaphore, 5);

		BSP_Line_CopyValues(sensorData);
		blackCount = 0;
		blackSum = 0;
		for (uint8_t i = 0; i < 24; i++) {
			if (sensorData[i] > blackLimit) {
				blackCount++;
				blackSum += i;
			}
		}

		if (osMutexWait(posMutex, 1)) {
			linePos = SENSOR_DIST_MM * ((blackSum / (float) blackCount) - 11.5f);
			osMutexRelease(posMutex);
		}

		if (++printCount >= 10) {
			printCount = 0;
			if (printLineState == ENABLE) {
				strcpy(outputBuffer, "L,");
				for (uint8_t i = 0; i < 24; i++)
					outputBuffer[i + 2] = sensorValToHexDigit(sensorData[i]);
				strcpy(outputBuffer + 26, "\r\n");
				SendString(outputBuffer);
			}
		}

//		vbatm = BSP_Sensor_GetVBATM() * 9000 >> 12; //divide by 4096
//		vbate = BSP_Sensor_GetVBATE() * 9000 >> 12; //divide by 4096
//		if (printState == ENABLE) {
//			sprintf(outputBuffer, "B,%lu,%lu\r\n", vbate, vbatm);
//			SendString(outputBuffer);
//		}
	}
}

void setPrintBattery(char state) {
	if (state == '1') {
		printBatteryState = ENABLE;
	} else {
		printBatteryState = DISABLE;
	}
}

void setPrintLine(char state) {
	if (state == '1') {
		printLineState = ENABLE;
	} else {
		printLineState = DISABLE;
	}
}

float getLinePos() {
	float retval;
	if (osMutexWait(posMutex, 1)) {
		retval = linePos;
		osMutexRelease(posMutex);
	}
	return retval;
}

char sensorValToHexDigit(uint8_t val) {
	val >>= 4;
	if (val < 10)
		return val + '0';
	else
		return val - 10 + 'a';
}
