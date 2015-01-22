/*
 * encoder.h
 *
 *  Created on: 2014.07.24.
 *      Author: Gábor
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#define INCR_PER_METER 		8634.56f // 7.07343e4f
#define METER_PER_INCR	 	1.1581e-4f // 1.41374e-5f
#define TIME_STEP_MS		10
#define TIME_STEP			0.01f

#include "stm32f4xx.h"

void BSP_Encoder_Init(void);
int32_t BSP_Encoder_GetPosition(void);
int16_t BSP_Encoder_GetVelocity(void);
void BSP_Encoder_Reset(void);
void BSP_Encoder_TimerCallback(void);

#endif /* ENCODER_H_ */
