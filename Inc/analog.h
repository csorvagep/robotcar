/*
 * analog.h
 *
 *  Created on: 2014.08.17.
 *      Author: G�bor
 */

#pragma once

#include "stm32f4xx.h"
#include "cmsis_os.h"

void AnalogThread(void const * argument);
void setPrintBattery(char state);
float getLinePos(void);
