/*
 * motor.c
 *
 *  Created on: 2014.07.24.
 *      Author: Gábor
 */

#include "motor.h"

#define ENABLE_MOTOR()		do {														\
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, GPIO_PIN_SET);		\
								HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);	\
							} while(0)

#define DISABLE_MOTOR()		do {														\
								HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, GPIO_PIN_RESET);	\
								HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);	\
							} while(0)

static FunctionalState _enabled = DISABLE;
static FunctionalState _backlash = ENABLE;
static int16_t _speed = 0;

extern TIM_HandleTypeDef htim1;

/**
 * Initializes the motor driver module, and its variables.
 */
void BSP_Motor_Init(void)
{
	htim1.Instance->CCR1 = MIDDLE_VAL;
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
	DISABLE_MOTOR();
	_enabled = DISABLE;
	_speed = 0;
	_backlash = ENABLE;
}

/**
 * Sets the timer capture-compare register according to the speed parameter.
 * @param speed Speed of the motor between SPEED_MAX and -SPEED_MAX.
 */
void BSP_Motor_SetSpeed(int32_t speed)
{
	if(speed != _speed) {
		if(speed > SPEED_MAX) {
			_speed = SPEED_MAX;
		} else if(speed < -SPEED_MAX) {
			_speed = -SPEED_MAX;
		} else {
			_speed = (int16_t)speed;
		}
		if(_speed > -BACKLASH && _speed < BACKLASH) {
			_backlash = ENABLE;
			DISABLE_MOTOR();
		} else {
			_backlash = DISABLE;
			if(_enabled) {
				ENABLE_MOTOR();
			}
		}
		htim1.Instance->CCR1 = MIDDLE_VAL + _speed;
	}
}

/**
 * Sets the /SD pins of the motor driver ICs, according to state.
 * @param state
 */
void BSP_Motor_SetState(FunctionalState state)
{
	_enabled = state;
	if(_enabled && !_backlash) {
		ENABLE_MOTOR();
	} else {
		DISABLE_MOTOR();
	}
}
