/*
 * encoder.h
 *
 *  Created on: 2014.07.24.
 *      Author: Gábor
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#include "stm32f4xx.h"

void BSP_Encoder_Init(void);
int32_t BSP_Encoder_GetPosition(void);
int16_t BSP_Encoder_GetVelocity(void);
void BSP_Encoder_Reset(void);

#endif /* ENCODER_H_ */
