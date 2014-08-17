/**
 ******************************************************************************
 * File Name          : main.c
 * Date               : 10/08/2014 13:32:01
 * Description        : Main program body
 ******************************************************************************
 *
 * COPYRIGHT(c) 2014 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN 0 */
#include "encoder.h"
#include "radio.h"
#include "motor.h"
#include "sensor.h"

#include <stdio.h>

#include "communication.h"
#include "motorcontrol.h"
#include "analog.h"

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void StartThread(void const * argument);

int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration----------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* System interrupt init*/
	/* Sets the priority grouping field */
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC1_Init();
	MX_TIM1_Init();
	MX_TIM2_Init();
	MX_TIM4_Init();
	MX_TIM6_Init();
	MX_USART3_UART_Init();

	/* USER CODE BEGIN 2 */
	BSP_Motor_Init();
	BSP_Radio_Init();
	BSP_Encoder_Init();
	BSP_Sensor_Init();

	BSP_Radio_ConnectServo(ENABLE);
	BSP_Radio_ServoStatus(ENABLE);

	/* USER CODE END 2 */

	/* Code generated for FreeRTOS */
	/* Create Start thread */
	osThreadDef(USER_Thread, StartThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(USER_Thread), NULL);

	/* Start scheduler */
	osKernelStart(NULL, NULL);

	/* We should never get here as control is now taken by the scheduler */

	/* USER CODE BEGIN 3 */
	/* Infinite loop */
	while (1) {
	}
	/* USER CODE END 3 */

}

/** System Clock Configuration
 */
void SystemClock_Config(void) {

	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	__PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin __attribute__((unused))) {
	static FunctionalState enabled = DISABLE;
	enabled = !enabled;
	BSP_Motor_SetState(enabled);
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask __attribute__((unused)),
		signed char *pcTaskName __attribute__((unused))) {
	while (1) {
	}
}
/* USER CODE END 4 */

static void StartThread(void const * argument __attribute__((unused))) {

	/* USER CODE BEGIN 5 */

	/* Communication thread */
	osThreadDef(COMM_Thread, CommThread, osPriorityAboveNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(COMM_Thread), NULL);

	/* Motor controller thread */
	osThreadDef(MOTOR_Thread, MotorThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(MOTOR_Thread), NULL);

	osThreadDef(ANALOG_Thread, AnalogThread, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(ANALOG_Thread), NULL);

	/* Infinite loop */
	for (;;) {
		osDelay(250);

		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
	}

	/* USER CODE END 5 */

}

#ifdef USE_FULL_ASSERT

/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file __attribute__((unused)), uint32_t line __attribute__((unused))) {
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	while (1)
		;
	/* USER CODE END 6 */

}

#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
