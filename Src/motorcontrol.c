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
#include "communication.h"

#include <stdio.h>

#define MAX_OUTPUT	450
#define MIN_OUTPUT	0

#define ZERO_TRESHOLD	8

#define LOOKUP_MAX	7
static const int16_t lookUpUMax[LOOKUP_MAX] = { 42, 48, 94, 177, 251, 325, 390 };
static const int16_t lookUpYMax[LOOKUP_MAX] = { 46, 50, 70, 95, 120, 145, 170 };
static const float lookUpKMax[LOOKUP_MAX] = { 0.67, 0.434783, 0.301205, 0.337838, 0.337838, 0.384615, 0.384615 };
#define ZD 			0.992f
#define KC			1.9f //2.4851f //Tcl = 500ms

static float refV = 0.1f;
static float refPhi = 0.0f;
static osMutexId refMutex = NULL;

static FunctionalState remoteControllerState = DISABLE;
static volatile FunctionalState printState = DISABLE;

int32_t calculateU(int32_t uv);

void MotorThread(void const * argument __attribute__((unused))) {
	uint32_t previousWakeTime = osKernelSysTick();
	uint8_t once = 1, i = 0;
	float ek = 0, uc1 = 0, uc2 = 0, uc = 0;
	int32_t uk = 0, u = 0;
	int32_t radioVal = 0;
	char outputBuffer[32];
	uint8_t rcState = 0;

	refMutex = osMutexCreate(NULL);
	configASSERT(refMutex);

	for (;;) {
		osDelayUntil(&previousWakeTime, 10);
		if (!remoteControllerState) {
			/* Dead man switch check */
			int16_t motor = BSP_Radio_GetMotor();
			if ((motor > 100 || motor < -100) && refV != 0.0f) {
				if (!once) {
					once = 1;
					BSP_Motor_SetState(ENABLE);
					BSP_Radio_ConnectServo(DISABLE);
				}

				float incrRef = refV * INCR_PER_METER * TIME_STEP;

				int16_t encoderVelo = BSP_Encoder_GetVelocity();
				if (motor < 0)
					encoderVelo *= -1;

				ek = incrRef - encoderVelo;
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
				if (motor < 0)
					u *= -1;
				BSP_Motor_SetSpeed(u);
				BSP_Radio_SetPhi(refPhi);
			} else { //dead man switch off
				if (once) {
					once = 0;
					BSP_Motor_SetSpeed(0);
					BSP_Motor_SetState(DISABLE);
					BSP_Radio_ConnectServo(DISABLE);
					ek = 0;
					uc1 = 0;
					uc2 = 0;
					uc = 0;
					uk = 0;
					u = 0;
				}
			}

			if (++i > 10) {
				if (printState == ENABLE) {
					sprintf(outputBuffer, "M,%ld,%ld,%d\r\n", (int32_t) ek, u, BSP_Encoder_GetVelocity());
					SendString(outputBuffer);
				}
				i = 0;
			}
		} else { //remoteControllerState

			switch (rcState) {
			/* Init state */
			case 0:
				BSP_Motor_SetSpeed(0);
				BSP_Motor_SetState(DISABLE);
				BSP_Radio_ConnectServo(ENABLE);
				rcState = 1;
				break;

			/* Wait state */
			case 1:
				if(++i > 10) {
					rcState = 2;
					i = 0;
				}
				break;

			/* Output state */
			case 2:
				radioVal = BSP_Radio_GetMotor();
				radioVal >>= 1;
				if(radioVal > -5 && radioVal < 5) {

				}
				BSP_Motor_SetSpeed(radioVal);

				break;
			default:
				break;
			}

			if (!once) {
				once = 1;
				BSP_Motor_SetSpeed(0);
				BSP_Motor_SetState(ENABLE);
				BSP_Radio_ConnectServo(ENABLE);
			}
			if (++i > 10) {
				i = 0;
			}
		}
	}
}

int32_t calculateU(int32_t uv) {
	uint8_t i = 0;
	int8_t sign = 1;

	if (uv < 0) {
		sign = -1;
		uv *= -1;
	}

	if (uv < ZERO_TRESHOLD)
		return 0;

	while (i < LOOKUP_MAX && lookUpUMax[i] < uv) {
		i++;
	}

	if (i == 0) {
		uv = lookUpYMax[0];
	} else {
		uv = (int32_t) (lookUpKMax[i - 1] * (uv - lookUpUMax[i - 1]) + lookUpYMax[i - 1] + 0.5);
	}

	return uv * sign;
}

void setRemoteControllerState(FunctionalState state) {
	remoteControllerState = state;
}

FunctionalState getRemoteControllerState(void) {
	return remoteControllerState;
}

void setVelocity(float velocity) {
	if (velocity > MAX_VELOCITY) {
		refV = MAX_VELOCITY;
	} else if (velocity < -MAX_VELOCITY) {
		refV = -MAX_VELOCITY;
	} else {
		refV = velocity;
	}
}

void setPhi(float phi) {
	if (phi > MAX_PHI)
		refPhi = MAX_PHI;
	else if (phi < -MAX_PHI)
		refPhi = -MAX_PHI;
	else
		refPhi = phi;
}

void setPrintMotor(char state) {
	if (state == '1') {
		printState = ENABLE;
	} else {
		printState = DISABLE;
	}
}
