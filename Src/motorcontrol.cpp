/*
 * motorcontrol.c
 *
 *  Created on: 2014.07.29.
 *      Author: Gábor
 */

#include "motorcontrol.h"

extern "C" {
#include "motor.h"
#include "encoder.h"
#include "radio.h"
#include "communication.h"
#include "analog.h"
}

#include <KalmanFilter.h>
#include <MotorController.h>
#include <LookUpTable.h>

#include <stdio.h>

#define MAX_OUTPUT				5100
#define POLE					-0.9887f
#define ZERO					0.0113f
#define ZD 						-POLE
#define KC						3.0f

#define RC_FORWARD_TRESHOLD		25

#define LOOKUP_MAX				19
static const float lookUpU[LOOKUP_MAX] = { 0, 0.1, 45, 120, 190, 225, 310, 350, 430, 485, 550, 600, 690, 750, 805, 880,
		955, 1020, 1080 };
static const float lookUpY[LOOKUP_MAX] = { 0, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145,
		150, 155 };

static float refV = 0.0f;
static float refPhi = 0.0f;
static osMutexId refMutex = NULL;

static FunctionalState remoteControllerState = ENABLE;
static volatile FunctionalState printState = DISABLE;

typedef enum {
	rcInit = 0, rcGo, rcStop, rcBreak, mcInit, mcGo, mcStop
} mcState_Typedef;

void MotorThread(void const * argument __attribute__((unused))) {
	/* Controller */
	MotorController controller(ZD, KC, MAX_OUTPUT);

	/* Kalman-filter */
	KalmanFilter filter(-POLE, 1, ZERO, 1.0f, 25.0f);

	/* Look-up table */
	LookUpTable table(lookUpU, lookUpY, LOOKUP_MAX);

	/* Common variables */
	uint32_t previousWakeTime = osKernelSysTick();
	char outputBuffer[64];
	FunctionalState prevRemoteState = remoteControllerState;

	/* RC-Mode variables */
	mcState_Typedef state = rcInit;

	/* Thread Init */
	refMutex = osMutexCreate(NULL);
	configASSERT(refMutex);

	BSP_Radio_ConnectServo(ENABLE);

	for (;;) {
		osDelayUntil(&previousWakeTime, 10);

		/* Input - Measurement phase */
		int16_t motor = BSP_Radio_GetMotor();
		int16_t incV = BSP_Encoder_GetVelocity();

		/* Check state switch */
		if (prevRemoteState != remoteControllerState) {
			if (remoteControllerState) {
				state = rcInit;
			} else {
				state = mcInit;
				//BSP_Radio_ConnectServo(DISABLE);
			}
			prevRemoteState = remoteControllerState;
		}

		switch (state) {
		case rcInit:
			BSP_Motor_SetSpeed(0);
			BSP_Motor_SetState(DISABLE);
			//BSP_Radio_ConnectServo(ENABLE);
			state = rcStop;
			break;

		case rcStop:
			if (motor > RC_FORWARD_TRESHOLD || motor < -RC_FORWARD_TRESHOLD) {
				BSP_Motor_SetState(ENABLE);
				state = rcGo;
			}
			break;

		case rcGo:
			if (motor < RC_FORWARD_TRESHOLD && motor > -RC_FORWARD_TRESHOLD) {
				state = rcBreak;
				BSP_Motor_SetSpeed(0);
			} else {
				BSP_Motor_SetSpeed(motor);
			}

			break;

		case rcBreak:
			if (motor > RC_FORWARD_TRESHOLD || motor < -RC_FORWARD_TRESHOLD) {
				state = rcGo;
			} else if (incV == 0) {
				BSP_Motor_SetState(DISABLE);
				state = rcStop;
			}
			break;

		case mcInit:
			BSP_Motor_SetState(DISABLE);
			BSP_Motor_SetSpeed(0);
			//BSP_Radio_ConnectServo(DISABLE);
			state = mcStop;
			break;

		case mcStop:
			if (motor > 100) {
				state = mcGo;
				controller.Reset();
				controller.SetVRef(refV);
				filter.Reset();
				BSP_Motor_SetState(ENABLE);
			}
			break;

		case mcGo:
			if (motor < 100) {
				state = mcStop;
				BSP_Motor_SetState(DISABLE);
				BSP_Motor_SetSpeed(0);
			} else {
				float vFiltered = filter.Filter((float) incV, controller.GetU());
				float u = controller.Control(vFiltered);
				int32_t uv = (int32_t) table.Look(u);
				BSP_Motor_SetSpeed(uv);
			}
			break;

		default:
			state = rcInit;
			break;
		}

		//if (!remoteControllerState) {
		//	BSP_Radio_SetPhi(refPhi);
		//}

		//if (++i > 5) {
//			if (printState == ENABLE) {
//				sprintf(outputBuffer, "M,%.2f,%.2f,%.2f,%d\r\n", ek, uc1, uc2, encoderVelo);
//				SendString(outputBuffer);
//			}
		//}
	}
}

extern "C" {
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
}
