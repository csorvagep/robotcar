/**
  ******************************************************************************
  * File Name          : main.c
  * Date               : 24/07/2014 16:07:47
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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN 0 */
#include "bluetooth.h"
#include <stdio.h>

__IO uint32_t uwIC2Value = 0;
__IO uint32_t uwIC1Value = 0;

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

int main(void)
{

  /* USER CODE BEGIN 1 */
	//uint32_t currentPosition;
	//int8_t dutyDelta = 0;
	//uint8_t direction = 0;

	char str_buff[128];

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* System interrupt init*/
  /* Sets the priority grouping field */
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();

  /* USER CODE BEGIN 2 */
	BSP_BT_Init();
	BSP_Motor_Init();

	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_1);

	HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_3);

  /* USER CODE END 2 */

  /* USER CODE BEGIN 3 */
	/* Infinite loop */
	while (1) {
		HAL_Delay(500);
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);

		sprintf(str_buff, "%lu,%lu\r\n", uwIC1Value, uwIC2Value);
		BSP_BT_SendStr(str_buff);
		BSP_BT_Flush();

		//HAL_ADC_Start(&hadc1);
		//HAL_ADC_PollForConversion(&hadc1, 10);
		//if(HAL_ADC_GetState(&hadc1) == HAL_ADC_STATE_EOC_REG) {
		//	currentPosition = HAL_ADC_GetValue(&hadc1);
		//}

//		currentPosition = htim2.Instance->CNT;
//
//		if(direction) {
//			if(dutyDelta <= -30) {
//				direction = 0;
//			} else {
//				dutyDelta--;
//			}
//		} else {
//			if(dutyDelta >= 30) {
//				direction = 1;
//			} else {
//				dutyDelta++;
//			}
//		}
//		htim1.Instance->CCR1 = 100 + dutyDelta;
	}
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

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

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

}

/* USER CODE BEGIN 4 */

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM4) {
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			uwIC1Value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
		} else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
			uwIC2Value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3) - uwIC1Value;
		}
	}
}

/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
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
