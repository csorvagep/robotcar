/*
 * linesensor.h
 *
 *  Created on: 2014 nov. 16
 *      Author: Gábor
 */

#pragma once

#include "cmsis_os.h"
#include "stm32f4xx.h"

void BSP_Line_Init(void);
void BSP_Line_TimerCallback(void);
void BSP_Line_StartMeasure(osSemaphoreId semaphoreID);
void BSP_Line_CopyValues(uint8_t* buffer);
