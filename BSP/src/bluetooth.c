/*
 * bluetooth.c
 *
 *  Created on: 2014.07.11.
 *      Author: Gábor
 */

#include "bluetooth.h"

#define RX_BUFFER_SIZE 64
#define TX_BUFFER_SIZE 256

uint32_t rxPosition = 0;
__IO ITStatus rxOverflow = RESET;
uint8_t rxBuff[RX_BUFFER_SIZE];

uint8_t txBuff1[TX_BUFFER_SIZE];
uint8_t txBuff2[TX_BUFFER_SIZE];
uint16_t txPosition = 0;

uint8_t *txCurrentBuff = txBuff1;

extern DMA_HandleTypeDef hdma_usart3_rx;

/* Public functions */
void BSP_BT_Init(void) {
	HAL_UART_Receive_DMA(&huart3, rxBuff, RX_BUFFER_SIZE);
}

void BSP_BT_SendStr(char *str) {
	while (*str) {
		txCurrentBuff[txPosition++] = *str++;
		if (txPosition >= TX_BUFFER_SIZE) {
			BSP_BT_Flush();
		}
	}
}

void BSP_BT_SendChars(char *str, size_t len) {
	while (len--) {
		txCurrentBuff[txPosition++] = *str++;
		if (txPosition >= TX_BUFFER_SIZE) {
			BSP_BT_Flush();
		}
	}
}

uint8_t BSP_BT_ReceiveStr(char *buffer, uint8_t buffer_size) {
	uint8_t i = 0;
	uint8_t currentPosition = (uint8_t)(RX_BUFFER_SIZE - hdma_usart3_rx.Instance->NDTR);

	while((rxOverflow == SET || rxPosition != currentPosition) && i < buffer_size - 1) {
		buffer[i++] = rxBuff[rxPosition++];
		if(rxPosition >= RX_BUFFER_SIZE) {
			rxOverflow = RESET;
			rxPosition = 0;
		}
	}
	buffer[i] = '\0';
	return i;
}

uint8_t BSP_BT_ReceiveStrNL(char *buffer, uint8_t buffer_size) {
	uint8_t cr = 0, lf = 0;
	uint8_t i = 0;

	uint32_t currentPosition = (uint8_t)(RX_BUFFER_SIZE - hdma_usart3_rx.Instance->NDTR);
	uint32_t position = rxPosition;
	uint8_t isOVF = rxOverflow;

	/* Go through the buffer, looking for a '\r\n' combination. */
	while((isOVF || position != currentPosition) && i < buffer_size && !lf) {
		if(cr && rxBuff[position] == '\n') {
			lf = 1;
		} else if(cr && rxBuff[position] != '\n') {
			cr = 0;
		} else if(rxBuff[position] == '\r') {
			cr = 1;
		}
		position++;
		i++;

		if(position >= RX_BUFFER_SIZE) {
			isOVF = 0;
			position = 0;
		}
	}

	if(lf) {
		return BSP_BT_ReceiveStr(buffer, i + 1);
	} else {
		return 0;
	}
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
