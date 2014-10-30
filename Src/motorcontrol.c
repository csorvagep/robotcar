/*
 * motorcontrol.c
 *
 *  Created on: 2014.07.29.
 *      Author: Gábor
 */

#include "motorcontrol.h"

#include "motor.h"
#include "encoder.h"
#include "radio.h"

//#include <stdio.h>

#define MAX_OUTPUT	450
#define MIN_OUTPUT	-450

#define LOOKUP_MAX	6
static const int16_t lookUpUMax[LOOKUP_MAX] = { 48, 94, 177, 251, 325, 390 };
static const int16_t lookUpYMax[LOOKUP_MAX] = { 50, 70, 95, 120, 145, 170 };
static const float lookUpKMax[LOOKUP_MAX] = { 0.434783, 0.301205, 0.337838, 0.337838, 0.384615, 0.384615 };
#define ZD 			0.992f
#define KC			1.9f //2.4851f //Tcl = 500ms
static float ref = 1.0f;
static osMutexId refMutex = NULL;

static FunctionalState remoteControllerState = DISABLE;

int32_t calculateU(int32_t uv);

int32_t calculateU(int32_t uv) {
	uint8_t i = 0;
	int8_t sign = 1;

	if (uv < 0) {
		sign = -1;
		uv *= -1;
	}

	while (i < LOOKUP_MAX && lookUpUMax[i] < uv) {
		i++;
	}

	if (i != 0) {
//		uv = lookUpYMax[0];
//	} else {
		uv = (int32_t) (lookUpKMax[i - 1] * (uv - lookUpUMax[i - 1]) + lookUpYMax[i - 1] + 0.5);
	}

	return uv * sign;
}

void MotorThread(void const * argument __attribute__((unused))) {
	uint32_t previousWakeTime = osKernelSysTick();
	uint8_t once = 1, i = 0;
	float ek = 0, uc1 = 0, uc2 = 0, uc = 0;
	int32_t uk = 0, u = 0;
	int32_t radioVal = 0;

	refMutex = osMutexCreate(NULL);
	configASSERT(refMutex);

	//BSP_Motor_SetBreakState(ENABLE);

	for (;;) {
		osDelayUntil(&previousWakeTime, 10);
		if (!remoteControllerState) {
			/* Dead man switch check */
			int16_t motor = BSP_Radio_GetMotor();
			if ((motor > 100 || motor < -100) && ref != 0.0f) {
				if (!once) {
					once = 1;
					BSP_Motor_SetState(ENABLE);
				}

				float incrRef = ref * INCR_PER_METER * TIME_STEP;
				if(motor < 0)
					incrRef *= -1.0f;

				ek = incrRef - BSP_Encoder_GetVelocity();
				uc2 = uc2 * ZD + (1 - ZD) * uk;
				uc1 = KC * ek;
				uc = uc1 + uc2;
				if (uc > MAX_OUTPUT) {
					uk = MAX_OUTPUT;
				} else if (uc < MIN_OUTPUT) {
					uk = MIN_OUTPUT;
				} else {
					uk = (int32_t) uc;
				}

				u = calculateU(uk);
				BSP_Motor_SetSpeed(u);
				//printf("M,%ld,%ld,%d\r\n", (int32_t) ek, u, BSP_Encoder_GetVelocity());
			} else {
				if (once) {
					once = 0;
					BSP_Motor_SetSpeed(0);
					BSP_Motor_SetState(DISABLE);
					ek = 0;
					uc1 = 0;
					uc2 = 0;
					uc = 0;
					uk = 0;
					u = 0;
				}
			}

			//if (++i > 10) {
			//	printf("M,%ld,%ld,%d\r\n", (int32_t) ek, u, BSP_Encoder_GetVelocity());
			//	i = 0;
			//}
		} else {
			if (!once) {
				once = 1;
				BSP_Motor_SetSpeed(0);
				BSP_Motor_SetState(ENABLE);
			}
			if (++i > 10) {
				radioVal = BSP_Radio_GetMotor();
				radioVal /= 3;
//				if(radioVal < -10) {
//					BSP_Motor_SetBreak((uint16_t)(radioVal * -1));
//					BSP_Motor_SetBreakState(ENABLE);
//				} else {
//					BSP_Motor_SetBreakState(DISABLE);
					BSP_Motor_SetSpeed(radioVal);
//				}
				i = 0;
			}
		}
	}
}

void setRemoteControllerState(FunctionalState state) {
	remoteControllerState = state;
}

FunctionalState getRemoteControllerState(void) {
	return remoteControllerState;
}
