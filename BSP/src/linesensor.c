/*
 * linesensor.c
 *
 *  Created on: 2014 nov. 16
 *      Author: Gábor
 */

#include "linesensor.h"

#include "spi.h"
#include "tim.h"

#define SPI_LE_ON()		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET)
#define SPI_LE_OFF()	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET)
#define SPI_OE_ON()		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
#define SPI_OE_OFF()	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)

#define SEQ_LEN		13
static const uint16_t LED_Sequence[SEQ_LEN] = { 0x0800, 0x0400, 0x0200, 0x0100, 0x0080, 0x0040, 0x0020, 0x0010, 0x0008,
		0x0004, 0x0002, 0x0001, 0x0000 };
static uint8_t sendCounter = 0;

void BSP_Line_StartMeasure(void) {
	sendCounter = 0;
	SPI_LE_ON();
	SPI_OE_ON();
	HAL_SPI_Transmit_IT(&hspi2, (uint8_t *) (LED_Sequence + sendCounter++), 2);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi->Instance == hspi2.Instance) {
		SPI_LE_OFF();
		htim5.Instance->CNT = 0;
		HAL_TIM_Base_Start_IT(&htim5);
	}
}

void BSP_Line_TimerCallback(void) {
	if (sendCounter < SEQ_LEN) {
		SPI_LE_ON();
		HAL_SPI_Transmit_IT(&hspi2, (uint8_t *) (LED_Sequence + sendCounter++), 2);
	} else {
		SPI_OE_OFF();
		//TODO jelezni hogy vége
	}
	HAL_TIM_Base_Stop_IT(&htim5);
}
