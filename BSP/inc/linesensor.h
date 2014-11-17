/*
 * linesensor.h
 *
 *  Created on: 2014 nov. 16
 *      Author: Gábor
 */

#pragma once

#include "cmsis_os.h"

void BSP_Line_TimerCallback(void);
void BSP_Line_StartMeasure(osSemaphoreId semaphoreID);
