/*
 * sensor.h
 *
 *  Created on: 2014.07.29.
 *      Author: Gábor
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include "stm32f4xx.h"

void BSP_Sensor_Init(void);
uint16_t BSP_Sensor_GetVBATM(void);
uint16_t BSP_Sensor_GetVBATE(void);

#endif /* SENSOR_H_ */
