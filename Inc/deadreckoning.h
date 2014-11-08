#pragma once

#include "stm32f4xx.h"

void DeadReckoningThread(void const * argument);
void setPrintConfig(char state);
void printConfig(void);
void resetConfig(void);
void setConfig(float newX, float newY, float newTheta);
