/*
 * radio.h
 *
 *  Created on: 2014.07.24.
 *      Author: Gábor
 */

#ifndef RADIO_H_
#define RADIO_H_

#include "stm32f4xx.h"

void BSP_Radio_Init(void);
int16_t BSP_Radio_GetSteer(void);
int16_t BSP_Radio_GetMotor(void);
void BSP_Radio_SetSteer(int16_t value);
void BSP_Radio_ServoStatus(FunctionalState state);
void BSP_Radio_ConnectServo(FunctionalState state);

#endif /* RADIO_H_ */
