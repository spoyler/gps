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
void Check_Debug_Mode();
uint8_t Get_Debug_State(uint8_t debug_bit_possition);
void Stop_Debug_Mode();
void Debug_Task();
void LPUART1_IRQHandler(void);
void Reset_Debug_State(uint8_t bit_position);

//const char debug_ccommands[debug_commands_size][32] = {{"dbg"}, {"toggle"}, {"gps"}, {"gsm"}, {"acc"}, {"lamp"}, {"volt"}, {"id"}};
enum debug_bit_possition
{
	DBG = 0,
	TOGGLE = 1,
	GPS = 2,
	GSM = 3,
	ACC = 4,
	LAMP_ON = 5,
	LAMP_OFF = 6,
	CHARG = 7,
	ID = 8,
	SLEEP = 9,
	HELP = 10,
	STOP = 11
};


#endif
