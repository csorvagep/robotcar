/*
 * motor.h
 *
 *  Created on: 2014.07.24.
 *      Author: Gábor
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "stm32f4xx.h"

#define SPEED_MAX		200
#define BACKLASH		20
#define MIDDLE_VAL		250
#define BREAK_PERIOD	250
#define BREAK_MAX		200

void BSP_Motor_Init(void);
void BSP_Motor_SetSpeed(int32_t value);
void BSP_Motor_SetState(FunctionalState state);
void BSP_Motor_SetBreakState(FunctionalState state);
void BSP_Motor_SetBreak(uint16_t value);
void BSP_Motor_BreakCallback(void);

#endif /* MOTOR_H_ */
