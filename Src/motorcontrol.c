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

#define MAX_OUTPUT				450
#define MIN_OUTPUT				-450

#define ZD 						0.992f
#define KC						2.4851f //Tcl = 500ms#define ZERO_TRESHOLD			8
#define RC_FORWARD_TRESHOLD		25
#define RC_1_MOTOR_MAX			0.002f

#define LOOKUP_MAX				7
static const int16_t lookUpUMax[LOOKUP_MAX] = { 42, 48, 94, 177, 251, 325, 390 };
static const int16_t lookUpYMax[LOOKUP_MAX] = { 46, 50, 70, 95, 120, 145, 170 };
static const float lookUpKMax[LOOKUP_MAX] = { 0.67, 0.434783, 0.301205, 0.337838, 0.337838,
		0.384615, 0.384615 };

static float refV = 0.0f;
static float refPhi = 0.0f;
static osMutexId refMutex = NULL;

static FunctionalState remoteControllerState = ENABLE;
static volatile FunctionalState printState = DISABLE;

typedef enum {
	rcInit = 0, rcForward, rcBreak, rcStop, mcOn, mcOff
} mcState_Typedef;

int32_t calculateU(int32_t uv);

void MotorThread(void const * argument __attribute__((unused)))
{

	/* Common variables */
	uint32_t previousWakeTime = osKernelSysTick();
	uint8_t i = 0;
	char outputBuffer[32];
	FunctionalState prevRemoteState = remoteControllerState;
	float v;

	/* MotorControl-Mode variables */
	float ek = 0, uc1 = 0, uc2 = 0, uc = 0;
	int32_t uk = 0, u = 0;

	/* RC-Mode variables */
	mcState_Typedef state = rcInit;
	int16_t prevMotor = 0;

	/* Thread Init */
	refMutex = osMutexCreate(NULL);
	configASSERT(refMutex);

	for(;;) {
		osDelayUntil(&previousWakeTime, 10);
		int16_t motor = BSP_Radio_GetMotor();

		/* Check state switch */
		if(prevRemoteState != remoteControllerState) {
			if(remoteControllerState) {
				state = rcInit;
				//BSP_Radio_ConnectServo(ENABLE);
			} else {
				state = mcOff;
				BSP_Motor_SetState(DISABLE);
				BSP_Motor_SetSpeed(0);
				//BSP_Radio_ConnectServo(DISABLE);
			}
			prevRemoteState = remoteControllerState;
		}

		switch(state) {
			case rcInit:
				BSP_Motor_SetSpeed(0);
				BSP_Motor_SetState(DISABLE);
				BSP_Radio_ConnectServo(ENABLE);
				state = rcStop;
				prevMotor = 0;
				break;

			case rcStop:
				if(motor > RC_FORWARD_TRESHOLD || motor < -RC_FORWARD_TRESHOLD) {
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
				if(prevMotor > RC_FORWARD_TRESHOLD && motor < -RC_FORWARD_TRESHOLD) {
					state = rcBreak;
				} else if(motor < RC_FORWARD_TRESHOLD && motor > -RC_FORWARD_TRESHOLD) {
					state = rcStop;
					BSP_Motor_SetState(DISABLE);
				} else {
					v = motor * RC_1_MOTOR_MAX * MAX_VELOCITY;
				}
				prevMotor = motor;
				break;

			case rcBreak:
				v = 0.0f;
				if(motor < RC_FORWARD_TRESHOLD && motor > -RC_FORWARD_TRESHOLD) {
					state = rcStop;
					BSP_Motor_SetState(DISABLE);
				}
				break;

			case mcOff:
				if(motor > 100) {
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
				if(motor < 100) {
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
		if(v > MAX_VELOCITY)
			v = MAX_VELOCITY;
		else if(v < -MAX_VELOCITY)
			v = -MAX_VELOCITY;

		/* MotorControl */
		float incrRef = v * INCR_PER_METER * TIME_STEP;

		int16_t encoderVelo = BSP_Encoder_GetVelocity();

		ek = incrRef - encoderVelo;
		uc2 = uc2 * ZD + (1 - ZD) * uk;
		uc1 = KC * ek;
		uc = uc1 + uc2;
		if(uc > MAX_OUTPUT) {
			uk = MAX_OUTPUT;
		} else if(uc < MIN_OUTPUT) {
			uk = MIN_OUTPUT;
		} else {
			uk = (int32_t) uc;
		}

		u = calculateU(uk);
		BSP_Motor_SetSpeed(u);

		if(!remoteControllerState)
			BSP_Radio_SetPhi(refPhi);

		if(++i > 10) {
			if(printState == ENABLE) {
				sprintf(outputBuffer, "M,%ld,%ld,%d\r\n", (int32_t) ek, u,
						BSP_Encoder_GetVelocity());
				SendString(outputBuffer);
			}
			i = 0;
		}

	}
}

int32_t calculateU(int32_t uv)
{
	uint8_t i = 0;
	int8_t sign = 1;

	if(uv < 0) {
		sign = -1;
		uv *= -1;
	}

	if(uv < ZERO_TRESHOLD)
		return 0;

	while(i < LOOKUP_MAX && lookUpUMax[i] < uv) {
		i++;
	}

	if(i == 0) {
		uv = lookUpYMax[0];
	} else {
		uv = (int32_t) (lookUpKMax[i - 1] * (uv - lookUpUMax[i - 1]) + lookUpYMax[i - 1] + 0.5);
	}

	return uv * sign;
}

void setRemoteControllerState(FunctionalState state)
{
	remoteControllerState = state;
}

FunctionalState getRemoteControllerState(void)
{
	return remoteControllerState;
}

void setVelocity(float velocity)
{
	if(velocity > MAX_VELOCITY) {
		refV = MAX_VELOCITY;
	} else if(velocity < -MAX_VELOCITY) {
		refV = -MAX_VELOCITY;
	} else {
		refV = velocity;
	}
}

void setPhi(float phi)
{
	if(phi > MAX_PHI)
		refPhi = MAX_PHI;
	else if(phi < -MAX_PHI)
		refPhi = -MAX_PHI;
	else
		refPhi = phi;
}

void setPrintMotor(char state)
{
	if(state == '1') {
		printState = ENABLE;
	} else {
		printState = DISABLE;
	}
}
