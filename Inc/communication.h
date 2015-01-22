/*
 * communication.h
 *
 *  Created on: 2014.07.28.
 *      Author: Gábor
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include "stm32f4xx.h"
#include "cmsis_os.h"

#define USE_SPP 	0

void CommThread(void const * argument);
void SendChars(const char * str, size_t len);
void SendString(char* str);

#endif /* COMMUNICATION_H_ */
