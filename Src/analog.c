/*
 * analog.c
 *
 *  Created on: 2014.08.17.
 *      Author: Gábor
 */

#include "analog.h"
#include "sensor.h"

#include "communication.h"
#include <stdio.h>

#include "spi.h"

static volatile FunctionalState printState = DISABLE;

#define SEQ_LEN		13
static const uint16_t LED_Sequence[SEQ_LEN] = { 0x0800, 0x0400, 0x0200, 0x0100, 0x0080, 0x0040, 0x0020, 0x0010, 0x0008, 0x0004, 0x0002, 0x0001, 0x0000 };

void AnalogThread(void const * argument __attribute__((unused))) {
	uint32_t previousWakeTime = osKernelSysTick();
	uint32_t vbatm, vbate;
	char outputBuffer[16];
	uint8_t i = 0;

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	for (;;) {
		osDelayUntil(&previousWakeTime, 750);

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
		HAL_SPI_Transmit_IT(&hspi2, (uint8_t *)(LED_Sequence + i++), 2);


		if(i >= SEQ_LEN) {
			i = 0;
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
		}

//		vbatm = BSP_Sensor_GetVBATM() * 9000 >> 12; //divide by 4096
//		vbate = BSP_Sensor_GetVBATE() * 9000 >> 12; //divide by 4096
//		if (printState == ENABLE) {
//			sprintf(outputBuffer, "B,%lu,%lu\r\n", vbate, vbatm);
//			SendString(outputBuffer);
//		}
	}
}

void setPrintBattery(char state) {
	if(state == '1') {
		printState = ENABLE;
	} else {
		printState = DISABLE;
	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	if(hspi->Instance == hspi2.Instance) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
	}
}
