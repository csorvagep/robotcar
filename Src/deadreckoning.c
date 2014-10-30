/*
 * deadreckoning.c
 *
 *  Created on: 2014 okt. 24
 *      Author: Gábor
 */

#include "deadreckoning.h"

#include <stdio.h>
#include <math.h>
#include "cmsis_os.h"
#include "stm32f4xx.h"
#include "stm32f4_discovery_accelerometer.h"
#include "stm32f4_discovery_gyroscope.h"
#include "communication.h"
#include "encoder.h"
#include "MadgwickAHRS.h"

#define HALF_CIRC			(1 / 180.0f)
#define DEG2RAD(x)			(float)(x * HALF_CIRC * M_PI)

static volatile FunctionalState printState = DISABLE;

void DeadReckoningThread(void const * argument __attribute__((unused))) {
	uint32_t previousWakeTime;

	int16_t accelero[3];
	float gyro[3];
	float x = 0.0f;
	float y = 0.0f;
	float theta = 0.0f;

	char outputBuffer[64];
	uint16_t i = 0;

	BSP_ACCELERO_Reset();
	BSP_GYRO_Reset();
	osDelay(300);
	beta = 0.033f;

	previousWakeTime = osKernelSysTick();
	for (;;) {
		osDelayUntil(&previousWakeTime, TIME_STEP_MS);

		BSP_ACCELERO_GetXYZ(accelero);
		BSP_GYRO_GetXYZ(gyro);

		int16_t v = BSP_Encoder_GetVelocity();
		float ds = v * METER_PER_INCR;

		x += ds * cosf(theta);
		y += ds * sinf(theta);

		for (int j = 0; j < 3; j++) {
			gyro[j] = DEG2RAD(gyro[j] * 0.001f);
		}

		MadgwickAHRSupdateIMU(gyro[0], gyro[1], gyro[2], accelero[0] * 1.0f, accelero[1] * 1.0f, accelero[2] * 1.0f);

		theta = atan2(2 * (q1 * q2 - q0 * q3), 2 * q0 * q0 - 1.0f - 2 * q1 * q1);

		if (++i > 10) {
			if (printState == ENABLE) {
				sprintf(outputBuffer, "C,%4.2f,%4.2f,%4.2f\r\n", x, y, theta);
				SendString(outputBuffer);
			}
			i = 0;
		}
	}
}

void setPrintConfig(char state) {
	if(state == '1') {
		printState = ENABLE;
	} else {
		printState = DISABLE;
	}
}
