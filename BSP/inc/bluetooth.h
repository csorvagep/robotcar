/*
 * bluetooth.h
 *
 *  Created on: 2014.07.11.
 *      Author: G�bor
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include "stm32f4xx_hal.h"
#include "usart.h"

void BSP_BT_Init(void);
void BSP_BT_SendStr(int8_t *str);
uint8_t BSP_BT_ReceiveStr(int8_t *buffer, uint8_t buffer_size);
void BSP_BT_Flush(void);
void BSP_BT_SetLed(void);
void BSP_BT_ResetLed(void);
void BSP_BT_ToggleLed(void);

#endif /* BLUETOOTH_H_ */
