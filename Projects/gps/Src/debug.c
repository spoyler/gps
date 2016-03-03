// Header:			debug.h
// File Name:  	debug.c
// Author:			andrey sidorov	
// Date:				21.02.201

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "debug.h"

#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

UART_HandleTypeDef LUart;				// debug uart

int InitDebugUart()
{
	int error_state = HAL_OK;
	
	__LPUART1_CLK_ENABLE();
	
	LUart.Instance = LPUART1;
	LUart.Init.BaudRate   = 115200;
	LUart.Init.WordLength = UART_WORDLENGTH_8B;
	LUart.Init.StopBits   = UART_STOPBITS_1;
	LUart.Init.Parity     = UART_PARITY_NONE;
	LUart.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	LUart.Init.Mode       = UART_MODE_TX_RX;
	LUart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
	error_state += HAL_UART_DeInit(&LUart);
 
  error_state += HAL_UART_Init(&LUart);
}

PUTCHAR_PROTOTYPE
{	
	while(HAL_IS_BIT_CLR(LUart.Instance->ISR, UART_FLAG_TXE));
	LUart.Instance->TDR = ch;
  return ch;
}


void debug_simm800(UART_HandleTypeDef * gsm_uart)
{
	while(1)
	{
		if (gsm_uart->Instance->ISR & UART_FLAG_RXNE)
			LUart.Instance->TDR = gsm_uart->Instance->RDR;
		if (LUart.Instance->ISR & UART_FLAG_RXNE)
			gsm_uart->Instance->TDR = LUart.Instance->RDR;
	}
}