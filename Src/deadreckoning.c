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

#define INCR_PER_METER 		8634.56f
#define METER_PER_INCR	 	1.1581e-4f
#define TIME_STEP_MS		10
#define TIME_STEP			0.01f

#define HALF_CIRC			(1 / 180.0f)
#define DEG2RAD(x)			(float)(x * HALF_CIRC * M_PI)

static float wrapAngle(float phi);

void DeadReckoningThread(void const * argument __attribute__((unused))) {
	uint32_t previousWakeTime;

	int16_t accelero[3];
	float gyro[3], gyroOffset = 0.0f;
	float x = 0.0f;
	float y = 0.0f;
	float theta = 0.0f;
	float ds;
	float dtheta;

	char outputBuffer[64];
	uint8_t init = 1;
	uint16_t i = 0;

	BSP_ACCELERO_Reset();
	BSP_GYRO_Reset();
	osDelay(300);

	previousWakeTime = osKernelSysTick();
	for (;;) {
		osDelayUntil(&previousWakeTime, TIME_STEP_MS);

		if (init) {
			BSP_GYRO_GetXYZ(gyro);
			gyroOffset += gyro[2];
			i++;

			if (i == 100) {
				i = 0;
				init = 0;
				gyroOffset *= 0.001f;
			}
		} else {
			BSP_ACCELERO_GetXYZ(accelero);
			BSP_GYRO_GetXYZ(gyro);

			int16_t v = BSP_Encoder_GetVelocity();

			if (v) {
				ds = v * METER_PER_INCR;
				dtheta = DEG2RAD((gyro[2] - gyroOffset) * 0.001f);

				theta = wrapAngle(dtheta * TIME_STEP * 0.5f + theta);
				x += ds * cosf(theta);
				y += ds * sinf(theta);
				theta = wrapAngle(dtheta * TIME_STEP * 0.5f + theta);
			}

			if (++i > 10) {
				sprintf(outputBuffer, "G,%d,%d,%4.2f\r\nC,%4.2f,%4.2f,%4.2f\r\n", accelero[1], accelero[0], dtheta, x,
						y, theta);
				SendString(outputBuffer);
				i = 0;
			}

		}
	}
}

static float wrapAngle(float phi) {
	while (phi > M_PI)
		phi -= M_PI;
	while (phi < -M_PI)
		phi += M_PI;

	return phi;
}
