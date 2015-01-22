/*
 * radio.h
 *
 *  Created on: 2014.07.24.
 *      Author: Gábor
 */

#ifndef RADIO_H_
#define RADIO_H_

#include "stm32f4xx.h"

#define SERVO_MAX			570
#define SERVO_MIN			-250
#define SERVO_OFFSET		160

#define RADIO_MOTOR_MAX		500

#define RAD_PER_DIGIT		6.996667e-4f
#define DIGIT_PER_RAD		1429.25f

void BSP_Radio_Init(void);
int16_t BSP_Radio_GetSteer(void);
int16_t BSP_Radio_GetMotor(void);
void BSP_Radio_SetSteer(int16_t value);
void BSP_Radio_SetPhi(float phi);
void BSP_Radio_ServoStatus(FunctionalState state);
void BSP_Radio_ConnectServo(FunctionalState state);

#endif /* RADIO_H_ */
