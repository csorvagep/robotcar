/*
 * motorcontrol.h
 *
 *  Created on: 2014.07.29.
 *      Author: Gábor
 */

#ifndef MOTORCONTROL_H_
#define MOTORCONTROL_H_

#include "stm32f4xx.h"
#include "cmsis_os.h"

#define MAX_VELOCITY	4.0f
#define MAX_PHI			0.3f

void setRemoteControllerState(FunctionalState state);
FunctionalState getRemoteControllerState(void);
void MotorThread(void const * argument);
void setVelocity(float velocity);
void setPhi(float phi);
void setPrintMotor(char state);

#endif /* MOTORCONTROL_H_ */
