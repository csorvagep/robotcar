/*
 * bluetooth.c
 *
 *  Created on: 2014.07.11.
 *      Author: Gábor
 */

#include "bluetooth.h"

#define BUFFER_SIZE 128

uint32_t rxPosition = 0;
__IO ITStatus rxOverflow = RESET;
uint8_t rxBuff[BUFFER_SIZE];

uint8_t txBuff1[BUFFER_SIZE];
uint8_t txBuff2[BUFFER_SIZE];
uint16_t txPosition = 0;

uint8_t *txCurrentBuff = txBuff1;

extern DMA_HandleTypeDef hdma_usart3_rx;

/* Public functions */
void BSP_BT_Init(void) {
	HAL_UART_Receive_DMA(&huart3, rxBuff, BUFFER_SIZE);
}

void BSP_BT_SendStr(int8_t *str) {
	while (*str) {
		txCurrentBuff[txPosition++] = *str++;
		if (txPosition >= BUFFER_SIZE) {
			BSP_BT_Flush();
		}
	}
}

uint8_t BSP_BT_ReceiveStr(int8_t *buffer, uint8_t buffer_size) {
	uint8_t i = 0;
	uint8_t currentPosition = (uint8_t)(BUFFER_SIZE - hdma_usart3_rx.Instance->NDTR);

	while((rxOverflow == SET || rxPosition != currentPosition) && i < buffer_size) {
		buffer[i++] = rxBuff[rxPosition++];
		if(rxPosition >= BUFFER_SIZE) {
			rxOverflow = RESET;
			rxPosition = 0;
		}
	}
	return i;
}

void BSP_BT_Flush(void) {
	HAL_UART_Transmit_DMA(&huart3, txCurrentBuff, txPosition);
	txPosition = 0;
	if (txCurrentBuff == txBuff1) {
		txCurrentBuff = txBuff2;
	} else {
		txCurrentBuff = txBuff1;
	}
}

void BSP_BT_SetLed(void) {
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);
}

void BSP_BT_ResetLed(void) {
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
}

void BSP_BT_ToggleLed(void) {
	HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_10);
}

/* Callback functions */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	rxOverflow = SET;
}
