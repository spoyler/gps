// Header:		
// File Name: debug.h
// Author:		andrey sidorov
// Date:			21.02.2016

#ifndef __DEBUG_H
#define __DEBUG_H

#include "stm32l0xx_hal.h"

#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
int InitDebugUart();
void debug_simm800(UART_HandleTypeDef * gsm_uart);


#endif
