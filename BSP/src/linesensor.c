/*
 * linesensor.c
 *
 *  Created on: 2014 nov. 16
 *      Author: Gábor
 */

#include "linesensor.h"

#include "spi.h"
#include "tim.h"
#include "adc.h"

#define SPI_LE_ON()		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET)
#define SPI_LE_OFF()	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET)
#define SPI_OE_ON()		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
#define SPI_OE_OFF()	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)

#define SEQ_LEN		13
static const uint16_t LED_Sequence[SEQ_LEN] = { 0x0800, 0x0400, 0x0200, 0x0100, 0x0080, 0x0040, 0x0020, 0x0010, 0x0008,
		0x0004, 0x0002, 0x0001, 0x0000 };
static uint8_t sendCounter = 0;

static uint32_t adcData[2] = {0};
static osSemaphoreId readySem = NULL;

void BSP_Line_StartMeasure(osSemaphoreId semaphoreID) {
	sendCounter = 0;
	SPI_LE_ON();
	SPI_OE_ON();
	HAL_SPI_Transmit_IT(&hspi2, (uint8_t *) (LED_Sequence + sendCounter++), 2);
	HAL_ADCEx_InjectedStart_IT(&hadc1);
	HAL_ADCEx_InjectedStart_IT(&hadc2);
	HAL_TIM_Base_Start(&htim5);
	readySem = semaphoreID;
	if(readySem)
		osSemaphoreWait(readySem, 1);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi->Instance == hspi2.Instance) {
		SPI_LE_OFF();
		htim5.Instance->CNT = 0;
	}
}

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc) {
	if(hadc->Instance == hadc1.Instance) {
		adcData[0] = HAL_ADCEx_InjectedGetValue(&hadc1, 1);
	} else if(hadc->Instance == hadc2.Instance) {
		adcData[1] = HAL_ADCEx_InjectedGetValue(&hadc2, 1);
		if (sendCounter < SEQ_LEN) {
			SPI_LE_ON();
			HAL_SPI_Transmit_IT(&hspi2, (uint8_t *) (LED_Sequence + sendCounter++), 2);
		} else {
			SPI_OE_OFF();
			HAL_ADCEx_InjectedStop_IT(&hadc1);
			HAL_ADCEx_InjectedStop_IT(&hadc2);
			if(readySem)
				osSemaphoreRelease(readySem);
		}
	}
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
	(void) hadc;
	while(1);
}
