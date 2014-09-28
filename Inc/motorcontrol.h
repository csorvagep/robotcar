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

void setRemoteControllerState(FunctionalState state);
FunctionalState getRemoteControllerState(void);
void MotorThread(void const * argument);

#endif /* MOTORCONTROL_H_ */
