/*
 * encoder.c
 *
 *  Created on: 2014.07.24.
 *      Author: Gábor
 */

#include "encoder.h"
#include "motor.h"

#include "radio.h"

static int32_t _total = 0;
static int16_t _velo = 0;

static int32_t _oldCnt = 0;
static int32_t _currCnt = 0;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim6;

/**
 * Initializes the encoder hardware.
 */
void BSP_Encoder_Init(void) {
	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
	HAL_TIM_Base_Start_IT(&htim6);
	_total = 0;
	_velo = 0;
}

/**
 * Get the encoder position
 * @return position
 */
int32_t BSP_Encoder_GetPosition(void) {
	return _total;
}

/**
 * Get the encoder velocity
 * @return velocity
 */
int16_t BSP_Encoder_GetVelocity(void) {
	return _velo;
}

/**
 * Reset the encoder hardware, and counter.
 */
void BSP_Encoder_Reset(void) {
	_total = 0;
	_velo = 0;
	htim2.Instance->CNT = 0;
}

/**
 * Callback function for periodical check, and measure velocity in every 10ms
 * @param htim Timer handle to determinate the trigger.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if(htim->Instance == TIM6) {
		_oldCnt = _currCnt;
		_currCnt = htim2.Instance->CNT;
		_velo = _currCnt - _oldCnt;
		_total += _velo;
	}
}
