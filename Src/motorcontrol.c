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
#include "analog.h"

#include <stdio.h>

#define MAX_OUTPUT				5100
#define MIN_OUTPUT				-5100

#define ZD 						0.9832f
#define KC						1.7f //1.2438f //Tcl = 500ms

#define ZERO_TRESHOLD			8
#define RC_FORWARD_TRESHOLD		25
#define RC_1_MOTOR_MAX			0.002f

#define LOOKUP_MAX				18
static const int16_t lookUpYMax[LOOKUP_MAX] = { 0, 45, 120, 190, 225, 310, 350, 430, 485, 550, 600, 690, 750, 805, 880,
		955, 1020, 1080 };
static const int16_t lookUpUMax[LOOKUP_MAX] = { 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125, 130, 135, 140,
		145, 150, 155 };
static const float lookUpKMax[LOOKUP_MAX] = { .111111, .066667, .071429, .142857, .058824, .125000, .062500, .090909,
		.076923, .100000, .055556, .083333, .090909, .066667, .066667, .076923, .083333, .083333 };

static float refV = 0.0f;
static float refPhi = 0.0f;
static osMutexId refMutex = NULL;

static FunctionalState remoteControllerState = DISABLE;
static volatile FunctionalState printState = DISABLE;

typedef enum {
	rcInit = 0, rcForward, rcBreak, rcStop, mcOn, mcOff
} mcState_Typedef;

int32_t calculateU(int32_t uv);

void MotorThread(void const * argument __attribute__((unused))) {

	/* Common variables */
	uint32_t previousWakeTime = osKernelSysTick();
	char outputBuffer[64];
	FunctionalState prevRemoteState = remoteControllerState;
	float v = 0.0f;
	int16_t velocities[4] = {0};
	uint8_t veloIndex = 0;

	/* MotorControl-Mode variables */
	float ek = 0, uc1 = 0, uc2 = 0, uc = 0;
	int32_t uk = 0, u = 0;

	/* RC-Mode variables */
	mcState_Typedef state = mcOff;
	int16_t prevMotor = 0;

	/* Thread Init */
	refMutex = osMutexCreate(NULL);
	configASSERT(refMutex);

	for (;;) {
		osDelayUntil(&previousWakeTime, 10);
		int16_t motor = BSP_Radio_GetMotor();

		/* Check state switch */
		if (prevRemoteState != remoteControllerState) {
			if (remoteControllerState) {
				state = rcInit;
			} else {
				state = mcOff;
				BSP_Motor_SetState(DISABLE);
				BSP_Radio_ConnectServo(DISABLE);
				BSP_Motor_SetSpeed(0);
			}
			prevRemoteState = remoteControllerState;
		}

		switch (state) {
		case rcInit:
			BSP_Motor_SetSpeed(0);
			BSP_Motor_SetState(DISABLE);
			BSP_Radio_ConnectServo(ENABLE);
			state = rcStop;
			prevMotor = 0;
			break;

		case rcStop:
			if (motor > RC_FORWARD_TRESHOLD || motor < -RC_FORWARD_TRESHOLD) {
				state = rcForward;
				ek = 0;
				uc1 = 0;
				uc2 = 0;
				uc = 0;
				uk = 0;
				u = 0;
				BSP_Motor_SetState(ENABLE);
			}
			break;

		case rcForward:
			if (prevMotor > RC_FORWARD_TRESHOLD && motor < -RC_FORWARD_TRESHOLD) {
				state = rcBreak;
			} else if (motor < RC_FORWARD_TRESHOLD && motor > -RC_FORWARD_TRESHOLD) {
				state = rcStop;
				BSP_Motor_SetState(DISABLE);
			} else {
				if (motor < 0)
					v = (motor + RC_FORWARD_TRESHOLD) * RC_1_MOTOR_MAX * MAX_VELOCITY;
				else
					v = (motor - RC_FORWARD_TRESHOLD) * RC_1_MOTOR_MAX * MAX_VELOCITY;
			}
			prevMotor = motor;
			break;

		case rcBreak:
			v = 0.0f;
			if (motor < RC_FORWARD_TRESHOLD && motor > -RC_FORWARD_TRESHOLD) {
				state = rcStop;
				BSP_Motor_SetState(DISABLE);
			}
			break;

		case mcOff:
			if (motor > 100) {
				state = mcOn;
				ek = 0;
				uc1 = 0;
				uc2 = 0;
				uc = 0;
				uk = 0;
				u = 0;
				BSP_Motor_SetState(ENABLE);
			}
			break;

		case mcOn:
			if (motor < 100) {
				state = mcOff;
				BSP_Motor_SetState(DISABLE);
				BSP_Motor_SetSpeed(0);
			}
			v = refV;
			break;

		default:
			break;
		}

		/* Limit speed */
		if (v > MAX_VELOCITY)
			v = MAX_VELOCITY;
		else if (v < -MAX_VELOCITY)
			v = -MAX_VELOCITY;

		/* MotorControl */
		float incrRef = v * INCR_PER_METER * TIME_STEP;

		velocities[veloIndex++] = BSP_Encoder_GetVelocity();
		if(veloIndex >= 4)
			veloIndex = 0;


		int16_t encoderVelo;
		for(uint8_t i = 0; i< 4; i++)
			encoderVelo += velocities[i];
		encoderVelo >>= 2;

		ek = incrRef - encoderVelo;
		uc2 = uc2 * ZD + (1 - ZD) * uk;

		/* Kotyogás */
		//if(v < 0.2f && v > -0.2f) {
		//	uc1 = KC * 0.3f * ek;
		//} else {
			uc1 = KC * ek;
		//}

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

		if (!remoteControllerState) {
			BSP_Radio_SetPhi(refPhi);
		}

		//if (++i > 5) {
			if (printState == ENABLE) {
				sprintf(outputBuffer, "M,%.2f,%.2f,%.2f,%d\r\n", ek, uc1, uc2, encoderVelo);
				SendString(outputBuffer);
			}
		//}

	}
}

int32_t calculateU(int32_t uv) {
	uint8_t i = 0;
	int8_t sign = 1;

	if (uv < 0) {
		sign = -1;
		uv *= -1;
	}

	while (i < LOOKUP_MAX && lookUpYMax[i] < uv) {
		i++;
	}

	if (i == 0) {
		uv = 0;
	} else {
		uv = (int32_t) (lookUpKMax[i - 1] * (uv - lookUpYMax[i - 1]) + lookUpUMax[i - 1] + 0.5f);
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
