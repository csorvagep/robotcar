/*
 * radio.c
 *
 *  Created on: 2014.07.24.
 *      Author: Gábor
 */

#include "radio.h"

static __IO uint32_t uwIC2Value = 0;
static __IO uint32_t uwIC1Value = 0;

extern TIM_HandleTypeDef htim4;

/**
 * Initializes the Input capture module to read the RC signals.
 */
void BSP_Radio_Init(void) {
	uwIC2Value = 0;
	uwIC1Value = 0;

	HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_3);
}

/**
 * Get the steer value.
 * @return Value can be between -500 and 500
 */
int16_t BSP_Radio_GetSteer(void) {
	return (int16_t)(uwIC1Value - 1500);
}

/**
 * Get the motor value.
 * @return Value can be between -500 and 500
 */
int16_t BSP_Radio_GetMotor(void) {
	return (int16_t)(uwIC2Value - 1500);
}

/**
 * Callback function implementation.
 * @param htim timer handle which trigger the callback
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM4) {
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			uwIC1Value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
		} else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
			uwIC2Value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3) - uwIC1Value;
		}
	}
}
