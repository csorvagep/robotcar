/*
 * radio.c
 *
 *  Created on: 2014.07.24.
 *      Author: Gábor
 */

#include "radio.h"

static __IO int32_t uwIC2Value = 0;
static __IO int32_t uwIC1Value = 0;

static __IO int32_t uwOCValue = 1500;
static FunctionalState _connected = DISABLE;
static FunctionalState _servo = DISABLE;

static ITStatus _signalDetected = RESET;

extern TIM_HandleTypeDef htim4;

/**
 * Initializes the Input capture module to read the RC signals.
 */
void BSP_Radio_Init(void) {
	uwIC2Value = 0;
	uwIC1Value = 0;

	HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_3);
}

/**
 * Get the steer value.
 * @return Value can be between -500 and 500
 */
int16_t BSP_Radio_GetSteer(void) {
	if (_signalDetected == SET)
		return uwIC2Value - 1500;
	else
		return 0;
}

/**
 * Get the motor value.
 * @return Value can be between -500 and 500
 */
int16_t BSP_Radio_GetMotor(void) {
	if (_signalDetected == SET)
		return uwIC1Value - 1500;
	else
		return 0;
}

void BSP_Radio_SetSteer(int16_t value) {
	value += SERVO_OFFSET;
	if(value > SERVO_MAX) {
		value = SERVO_MAX;
	} else if(value < SERVO_MIN) {
		value = SERVO_MIN;
	}
	uwOCValue = 1500 + value;
	htim4.Instance->CCR4 = uwOCValue;
}

void BSP_Radio_ServoStatus(FunctionalState state) {
	_servo = state;
	if (state == ENABLE && _signalDetected == SET) {
		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
	} else {
		HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_4);
	}
}

void BSP_Radio_ConnectServo(FunctionalState state) {
	_connected = state;
}

/**
 * Callback function implementation.
 * @param htim timer handle which trigger the callback
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	static uint8_t steerRise = 0;

	if (htim->Instance == TIM4) {

		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {

			/* TIM4 Channel 1 (Motor input) */
			uwIC1Value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
		} else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {

			/* TIM4 Channel 3 (Steer input) */
			uwIC2Value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3) - uwIC1Value;
			steerRise = 1;

			if (_signalDetected == RESET) {
				_signalDetected = SET;
				if (_servo == ENABLE)
					HAL_TIM_PWM_Start_IT(&htim4, TIM_CHANNEL_4);

			}

			/* Connect to servo output */
			if (_connected == ENABLE) {
				BSP_Radio_SetSteer(uwIC2Value - 1500);
			}
		} else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {

			/* TIM4 Channel 2 (Motor input, fall edge) */
			if (!steerRise && _signalDetected == SET) {
				_signalDetected = RESET;
				HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_4);
			} else if (steerRise) {
				steerRise = 0;
			}
		}
	}
}
