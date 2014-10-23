/*
 * motor.c
 *
 *  Created on: 2014.07.24.
 *      Author: Gábor
 */

#include "motor.h"
#include "gpio.h"

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

static FunctionalState _break = DISABLE;
static uint16_t _breakDuty = 50;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim7;

/**
 * Initializes the motor driver module, and its variables.
 */
void BSP_Motor_Init(void) {
	htim1.Instance->CCR1 = MIDDLE_VAL;
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
	DISABLE_MOTOR()
	;
	_enabled = DISABLE;
	_speed = 0;
	_backlash = ENABLE;
}

/**
 * Sets the timer capture-compare register according to the speed parameter.
 * @param speed Speed of the motor between SPEED_MAX and -SPEED_MAX.
 */
void BSP_Motor_SetSpeed(int32_t speed) {
	if (speed != _speed) {
		if (speed > SPEED_MAX) {
			_speed = SPEED_MAX;
		} else if (speed < -SPEED_MAX) {
			_speed = -SPEED_MAX;
		} else {
			_speed = (int16_t) speed;
		}
		if (_speed > -BACKLASH && _speed < BACKLASH) {
			_backlash = ENABLE;
			DISABLE_MOTOR()
			;
		} else {
			_backlash = DISABLE;
			if (_enabled) {
				ENABLE_MOTOR()
				;
			}
		}
		htim1.Instance->CCR1 = MIDDLE_VAL + _speed;
	}
}

/**
 * Sets the /SD pins of the motor driver ICs, according to state.
 * @param state
 */
void BSP_Motor_SetState(FunctionalState state) {
	if (!_break) {
		_enabled = state;
		if (_enabled && !_backlash) {
			ENABLE_MOTOR()
			;
		} else {
			DISABLE_MOTOR()
			;
		}
	}
}

void BSP_Motor_SetBreakState(FunctionalState state) {
	if (state == ENABLE && _break == DISABLE) {
//		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
//		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
//		HAL_TIM_Base_Start_IT(&htim7);

		HAL_TIM_PWM_MspDeInit(&htim1);
		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
		ENABLE_MOTOR()
		;
		_break = ENABLE;
	} else if (state == DISABLE && _break == ENABLE) {
		_break = DISABLE;
		HAL_TIM_PWM_MspInit(&htim1);
		BSP_Motor_Init();
	}
}

void BSP_Motor_SetBreak(uint16_t value) {
	if (value > BREAK_MAX) {
		_breakDuty = BREAK_MAX;
	} else {
		_breakDuty = value;
	}
}

void BSP_Motor_BreakCallback(void) {
	static uint16_t cnt = 0;
	if (_break == ENABLE) {
		if (++cnt >= BREAK_PERIOD) {
			cnt = 0;
			ENABLE_MOTOR()
			;
		} else if (cnt == _breakDuty) {
			DISABLE_MOTOR()
			;
		}
	}
}

