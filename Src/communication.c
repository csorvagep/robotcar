/*
 * communication.c
 *
 *  Created on: 2014.07.28.
 *      Author: G�bor
 */

#include "communication.h"

#include "bluetooth.h"
#include <string.h>

static osMutexId sendMutex = NULL;
static const char connectString[] = "AT+AB SPPConnect 000272cfc2e7\r\n";

#define BUFFSIZE	64
static char internalBuffer[BUFFSIZE];

static ITStatus BypassMode = SET;
static ITStatus Connected = SET;
static ITStatus SPPTry = RESET;

static void ProcessCommand(void);
static void ProcessATAB(void);
static void ConnectionDown(void);

void CommThread(void const * argument __attribute__((unused))) {
	uint32_t previousWakeTime = osKernelSysTick();
	uint16_t cnt = 0;

	BSP_BT_Init();

	sendMutex = osMutexCreate(NULL);
	configASSERT(sendMutex);

	BSP_BT_SendStr("AT+AB Bypass\r\n");
	BSP_BT_Flush();

	for (;;) {
		osDelayUntil(&previousWakeTime, 50);

		while (BSP_BT_ReceiveStrNL(internalBuffer, BUFFSIZE)) {
			ProcessCommand();
		}

		if (Connected && BypassMode) {
			/* Empty output buffer */
			if (osMutexWait(sendMutex, 3) == osOK) {
				BSP_BT_Flush();
				osMutexRelease(sendMutex);
			}
		} else if (!Connected && SPPTry == RESET && cnt++ > 200) {
			BSP_BT_SendStr(connectString);
			BSP_BT_Flush();
			cnt = 0;
			SPPTry = SET;
		}
	}
}

void SendChars(const char * str, size_t len) {
	if (!Connected || !BypassMode)
		return;

	if (sendMutex != NULL) {
		if (osMutexWait(sendMutex, 3) == osOK) {
			BSP_BT_SendChars(str, len);
			osMutexRelease(sendMutex);
		}
	}
}

void ProcessCommand(void) {
	switch (internalBuffer[0]) {
	case 'A':
		ProcessATAB();
		break;
	default:
		break;
	}
}

void ProcessATAB(void) {
	if (!strncmp(internalBuffer, "AT-AB ", 6)) {
		if (!strcmp(internalBuffer + 6, "-CommandMode-")) {
			BypassMode = RESET;
		} else if (!strncmp(internalBuffer + 6, "ConnectionUp", 12)) {
			Connected = SET;
			BSP_BT_SetLed();
		} else if (!strncmp(internalBuffer + 6, "-BypassMode-", 12)) {
			BypassMode = SET;
		} else if (!strncmp(internalBuffer + 6, "ErrFormat", 9)) {
			ConnectionDown();
		} else if (!strncmp(internalBuffer + 6, "ConnectionDown", 14)) {
			ConnectionDown();
		} else if (!strncmp(internalBuffer + 6, "SPPConnectionClosed", 19)) {
			SPPTry = RESET;
		}
	}
}

void ConnectionDown(void) {
	BypassMode = RESET;
	Connected = RESET;
	BSP_BT_ResetLed();
}
