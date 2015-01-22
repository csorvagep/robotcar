/*
 * motor.h
 *
 *  Created on: 2014.07.24.
 *      Author: Gábor
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "stm32f4xx.h"

#define SPEED_MAX		490
#define MIDDLE_VAL		500

void BSP_Motor_Init(void);
void BSP_Motor_SetSpeed(int32_t value);
void BSP_Motor_SetState(FunctionalState state);

#endif /* MOTOR_H_ */
