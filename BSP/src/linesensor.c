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

#define SPI_LE_ON()			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET)
#define SPI_LE_OFF()		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET)
#define SPI_OE_ON()			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
#define SPI_OE_OFF()		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)

#define AN_E_ON()			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET)
#define AN_E_OFF()			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET)
#define AN_SELECT( x )		GPIOE->ODR = ((GPIOE->ODR & 0x0fff) | (( x ) << 12))

#define SEQ_LEN		12

static const uint16_t LED_Sequence[SEQ_LEN] = { 0x0800, 0x0400, 0x0200, 0x0100, 0x0080, 0x0040, 0x0020, 0x0010, 0x0008,
		0x0004, 0x0002, 0x0001 };
//static const uint16_t LED_Sequence[SEQ_LEN] = { 0xffff };
static const uint8_t MPX_Sequence[SEQ_LEN] = { 11, 3, 9, 1, 14, 6, 12, 4, 10, 2, 8, 0 };
//static const uint8_t MPX_Sequence[SEQ_LEN] = { 13 };
static uint8_t sendCounter = 0;

static uint32_t adcData[SEQ_LEN << 1] = { 0 };
static osSemaphoreId readySem = NULL;

void BSP_Line_Init(void) {
	uint16_t spiInitVal = 0x0000;
	AN_E_OFF();
	SPI_OE_OFF();
	SPI_LE_ON();
	HAL_SPI_Transmit(&hspi2, (uint8_t *) (&spiInitVal), 2, HAL_MAX_DELAY);
	SPI_LE_OFF();

}

void BSP_Line_StartMeasure(osSemaphoreId semaphoreID) {
	sendCounter = 0;
	AN_E_ON();
	SPI_LE_ON();
	SPI_OE_ON();
	HAL_SPI_Transmit_IT(&hspi2, (uint8_t *) LED_Sequence, 2);
	HAL_ADCEx_InjectedStart_IT(&hadc1);
	HAL_ADCEx_InjectedStart_IT(&hadc2);
	HAL_TIM_Base_Start(&htim5);
	readySem = semaphoreID;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi->Instance == hspi2.Instance) {
		SPI_LE_OFF();
		htim5.Instance->CNT = 0;
		AN_SELECT(MPX_Sequence[sendCounter]);
	}
}

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc) {
	if (hadc->Instance == hadc1.Instance) {
		adcData[sendCounter] = HAL_ADCEx_InjectedGetValue(&hadc1, 1);
	} else if (hadc->Instance == hadc2.Instance) {
		adcData[sendCounter + SEQ_LEN] = HAL_ADCEx_InjectedGetValue(&hadc2, 1);
		sendCounter++;
		if (sendCounter < SEQ_LEN) {
			SPI_LE_ON();
			HAL_SPI_Transmit_IT(&hspi2, (uint8_t *) (LED_Sequence + sendCounter), 2);
		} else {
			SPI_OE_OFF();
			AN_E_OFF();
			HAL_ADCEx_InjectedStop_IT(&hadc1);
			HAL_ADCEx_InjectedStop_IT(&hadc2);
			if (readySem)
				osSemaphoreRelease(readySem);
		}
	}
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
	(void) hadc;
	while (1)
		;
}

void BSP_Line_CopyValues(uint8_t* buffer) {
	for (uint8_t i = 0; i < (SEQ_LEN << 1); i++)
		buffer[i] = (uint8_t)adcData[i];
}
