// Header:			debug.h
// File Name:  	debug.c
// Author:			andrey sidorov	
// Date:				21.02.201

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "gps.h"
#include "GPRS_Shield_Arduino.h"
#include "adc.h"
#include "accelero.h"
#include "command.h"

const uint32_t debug_time = 5000;
const uint32_t debug_message_max_size = 32;
const uint32_t debug_commands_size = 10;
const char debug_ccommands[debug_commands_size][8] = {{"\0"}, {"toggle"}, {"gps"}, {"gsm"}, 
																											{"acc"}, {"lamp_on"}, {"lamp_off"}, {"volt"}, 
																											{"id"}, {"help"}};

uint32_t debug_state = 0;
uint32_t debug_toggle_start_time = 0;
uint32_t debug_gps_start_time = 0;
uint32_t debug_adc_time = 0;
uint32_t debug_acc_time = 0;

#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

UART_HandleTypeDef LUart;				// debug uart
char debug_message_string[debug_message_max_size];
uint32_t debug_message_index = 0;

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
	
	DEBUG_PRINTF("\r\n\r\n\r\nSystem startup!\r\n");
		
	return 0;
}

PUTCHAR_PROTOTYPE
{	
	while(HAL_IS_BIT_CLR(LUart.Instance->ISR, UART_FLAG_TXE));
	LUart.Instance->TDR = ch;
  return ch;
}

void Write_Help_Message()
{
		DEBUG_PRINTF("Pleease enter \'Esc\' to exit from debug mode.\r\n");
		DEBUG_PRINTF("Debug mode. Enter command to view debug messages:\r\n");
		for (int i = 1; i < debug_commands_size; ++i)
		{
			DEBUG_PRINTF("%s\r\n", debug_ccommands[i]);
		}
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

void Check_Debug_Mode()
{
	__HAL_UART_ENABLE_IT(&LUart, UART_IT_ORE | UART_IT_RXNE);
	
	/* Process Unlocked */
	__HAL_UNLOCK(&LUart); 
	HAL_NVIC_EnableIRQ(LPUART1_IRQn);
	
	DEBUG_PRINTF("Pleease enter \'del\' to enter in debug mode:\r\n");
	uint32_t start_time = HAL_GetTick();
	
	while((HAL_GetTick() - start_time) < debug_time)
	{
		if (Get_Debug_State(DBG))
		{
			Write_Help_Message();
			break;
		}
	}
	if (!Get_Debug_State(DBG))
	{
		__HAL_UART_DISABLE_IT(&LUart, UART_IT_ORE | UART_IT_RXNE);
		
		/* Process Unlocked */
		__HAL_UNLOCK(&LUart); 
		HAL_NVIC_DisableIRQ(LPUART1_IRQn);
	}
}

void Debug_Task()
{
	// check debug toggle
	if (Get_Debug_State(TOGGLE))
	{
		if ((debug_toggle_start_time == 0)
			|| ((HAL_GetTick() - debug_toggle_start_time) > 500))
		{
			debug_toggle_start_time = HAL_GetTick();
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);
		}
	}
	// print messages from GPS
	if (Get_Debug_State(GPS))
	{
		GPS_Debug();
	}

	// print signal leven and SIM status
	if (Get_Debug_State(GSM))
	{
		if ((debug_gps_start_time == 0)
			|| ((HAL_GetTick() - debug_gps_start_time) > 1000 ))
		{
			debug_gps_start_time = HAL_GetTick();
			GSM_Debug();
		}
	}
		// print acc values
	if (Get_Debug_State(ACC))
	{
		if ((debug_acc_time == 0)
			|| ((HAL_GetTick() - debug_acc_time) > 1000 ))
		{
			debug_acc_time = HAL_GetTick();
			ACC_Debug();
		}
	}
	
	// Off / On Lamp
	if (Get_Debug_State(LAMP_ON))
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
		Reset_Debug_State(LAMP_ON);
	}
	if (Get_Debug_State(LAMP_OFF))
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		Reset_Debug_State(LAMP_OFF);
	}
	
	// print adc values
	if (Get_Debug_State(CHARG))
	{
		if ((debug_adc_time == 0)
			|| ((HAL_GetTick() - debug_adc_time) > 1000 ))
		{
			debug_adc_time = HAL_GetTick();
			ADC_Debug();
		}
	}
	
	// print tracker id
	if (Get_Debug_State(ID))
	{
		Command_Debug();
		Reset_Debug_State(ID);
	}
	
	// print help message
	if (Get_Debug_State(HELP))
	{
		Write_Help_Message();
		Reset_Debug_State(HELP);
	}
	
}

void Stop_Debug_Mode()
{
	// off led
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
	debug_toggle_start_time = 0;
	__HAL_UART_DISABLE_IT(&LUart, UART_IT_ORE | UART_IT_RXNE);
	
	/* Process Unlocked */
	__HAL_UNLOCK(&LUart); 
	HAL_NVIC_DisableIRQ(LPUART1_IRQn);
}

void RNG_LPUART1_IRQHandler(void)
{
	// check errors
	if(__HAL_UART_GET_IT(&LUart, UART_IT_ORE) != RESET) 
  { 
    __HAL_UART_CLEAR_IT(&LUart, UART_CLEAR_OREF);
    
    LUart.ErrorCode |= HAL_UART_ERROR_ORE;
    /* Set the UART state ready to be able to start again the process */
    LUart.State = HAL_UART_STATE_READY;
		
		return;
  }
	
	// check data
	if(HAL_IS_BIT_SET(LUart.Instance->ISR, UART_FLAG_RXNE))
	{
		char c = LUart.Instance->RDR;
		
		if (debug_message_index >= debug_message_max_size)
			debug_message_index = 0;

		debug_message_string[debug_message_index] = c;
		LUart.Instance->TDR  = c;
		
		if (debug_message_string[debug_message_index] == 127)
		{
			debug_state = (1 << 0) | 1;
			memset(debug_message_string, 0, debug_message_max_size);
			debug_message_index = 0;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
			return;
		}
		
		if (debug_message_string[debug_message_index] == 27)
		{
			Stop_Debug_Mode();
			debug_state = 0;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
		}
			
				
		if (debug_message_index > 0)
		{
			if ( debug_message_string[debug_message_index - 1] 	== '\r'
				&& debug_message_string[debug_message_index] 		  == '\n')
			{
				for (int i = 0; i < debug_commands_size; ++i)
				{
					if (strstr(debug_message_string, &debug_ccommands[i][0]) != 0)
					{						
						// set true status
						debug_state = (1 << i) | 1;
																		
						// exit form debug mode
						if (i == STOP)
						{
							Stop_Debug_Mode();
							debug_state = 0;
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
						}
					}
				}				
				
				memset(debug_message_string, 0, debug_message_max_size);
				debug_message_index = 0;
				
				return;
			}
		}
		debug_message_index++;
	}
}

uint8_t Get_Debug_State(uint8_t debug_bit_position)
{
	if ((debug_state & (1 << DBG)) && (debug_state & (1 << debug_bit_position))) 
		return 1;
	else
		return 0;
			
}

void Reset_Debug_State(uint8_t bit_position)
{
	debug_state &= ~(1 << bit_position);
}
