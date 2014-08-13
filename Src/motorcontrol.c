/*
 * motorcontrol.c
 *
 *  Created on: 2014.07.29.
 *      Author: Gábor
 */

#include "motorcontrol.h"

#include "motor.h"
#include "encoder.h"
#include "radio.h"

#include <stdio.h>

void MotorThread(void const * argument __attribute__((unused))) {
	uint32_t previousWakeTime = osKernelSysTick();
	uint8_t once = 1;
	uint8_t i = 0;

	int32_t ek = 0, ik = 0, y = 0;

	for (;;) {
		osDelayUntil(&previousWakeTime, 100);

		if (i++ > 5) {
			printf("ek: %ld ik: %ld yk: %ld\r\n", ek, ik, y);
			i = 0;
		}

		/*if (BSP_Radio_GetMotor() > 100) {
			if (!once) {
				once = 1;
				BSP_Motor_SetState(ENABLE);
			}

			y = BSP_Encoder_GetVelocity();
			ek = 75 - y;
			ik += (ek >> 4);
			BSP_Motor_SetSpeed(ik);
		} else {
			if (once) {
				once = 0;
				BSP_Motor_SetState(DISABLE);
				ik = 0;
			}
		}
		*/

		ek = BSP_Radio_GetMotor();
		ek >>= 2;
		BSP_Motor_SetSpeed(ek);
	}
}
