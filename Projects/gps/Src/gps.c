// Header:		gps.h
// File Name: gps.c
// Author:		andrey sidorov
// Date:			28.02.2016


#include <stdio.h>
#include <string.h>
#include "gps.h"
#include "debug.h"

UART_HandleTypeDef UartGPS;			// gps uart


#define MAX_MAX_COMMAND		 		4
#define GPS_MAX_MESSAGE_SIZE 	256

//
const char gps_answers[MAX_MAX_COMMAND][32] = {{"GGA"}, {"ZDA"}, {"PMTK001"}, {"RMC"}};
char gps_messages[MAX_MAX_COMMAND][GPS_MAX_MESSAGE_SIZE];
//
uint32_t messages_bit_status = 0;
//
char gps_tmp_string[GPS_MAX_MESSAGE_SIZE];
int gps_message_index = 0;
int gps_message_recived = 0;

const char set_output_msg[] = "$PMTK314";
const char set_sleep_mode[] = "$PMTK161,0";



void GPS_Init()
{
	__USART1_CLK_ENABLE();
  UartGPS.Instance        = USART1;

  UartGPS.Init.BaudRate   = 9600;
  UartGPS.Init.WordLength = UART_WORDLENGTH_8B;
  UartGPS.Init.StopBits   = UART_STOPBITS_1;
  UartGPS.Init.Parity     = UART_PARITY_NONE;
  UartGPS.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  UartGPS.Init.Mode       = UART_MODE_TX_RX;
  UartGPS.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
  HAL_UART_DeInit(&UartGPS);
  HAL_UART_Init(&UartGPS);
	
  __HAL_UART_ENABLE_IT(&UartGPS, UART_IT_ORE | UART_IT_RXNE);
	
	/* Process Unlocked */
	__HAL_UNLOCK(&UartGPS); 
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	
	Set_Output_Msg();
}

/**
  * @brief
  * @param
  * @retval
  */
void Set_Output_Msg()
{
	char gps_output_message[GPS_MAX_MESSAGE_SIZE];
	memset(gps_output_message, 0, GPS_MAX_MESSAGE_SIZE);
	
	int msg_pos = 0;
		
	char dont_use = 0;
	char rmc_pos = 1;
	char gga_pos = 3; 	// pos 3
	char zda_pos = 17;	// pos 17
	
	char output_msg_state[19] = {0};
	
	output_msg_state[rmc_pos] = 1;
	output_msg_state[gga_pos] = 1;
	output_msg_state[zda_pos] = 5;
	
	msg_pos += sprintf(gps_output_message, "%s", set_output_msg);
	
	for (int i = 0; i < 19; ++i)
	{
		msg_pos += sprintf(&gps_output_message[msg_pos],",%d", output_msg_state[i]);
	}
	
	char crc = GPS_Calc_CRC(gps_output_message, msg_pos);
	
	msg_pos += sprintf(&gps_output_message[msg_pos],"*%x\r\n", crc);
	
	DEBUG_PRINTF(gps_output_message);
		
	GPS_Send_Message(gps_output_message);
}

void SetGPSSleepMode()
{
	char gps_output_message[64];
	memset(gps_output_message, 0, 64);
	
	int msg_pos = 0;
		
	msg_pos += sprintf(gps_output_message, "%s", set_sleep_mode);
	
	char crc = GPS_Calc_CRC(gps_output_message, msg_pos);
	
	msg_pos += sprintf(&gps_output_message[msg_pos],"*%x\r\n", crc);
	
	DEBUG_PRINTF(gps_output_message);
		
	GPS_Send_Message(gps_output_message);
}

/**
  * @brief
  * @param
  * @retval
  */
char GPS_Calc_CRC(char * msg, int msg_size)
{
	char crc = 0;
	int i = 0;
	
	while((msg[i++] != '$') && (i < msg_size));
	
	
	for ( ; i < msg_size; ++i)
	{
		crc ^= msg[i];
	}
	
	return crc;
}


void GPS_Send_Message(char * msg)
{
	
	int msg_len = strlen(msg);
	
	for (int i = 0; i < msg_len; ++i)
	{
		while(HAL_IS_BIT_CLR(UartGPS.Instance->ISR, UART_FLAG_TXE));
		UartGPS.Instance->TDR = msg[i];
	}
}


void USART1_IRQHandler(void)
{
	
	// check errors
	if(__HAL_UART_GET_IT(&UartGPS, UART_IT_ORE) != RESET) 
  { 
    __HAL_UART_CLEAR_IT(&UartGPS, UART_CLEAR_OREF);
    
    UartGPS.ErrorCode |= HAL_UART_ERROR_ORE;
    /* Set the UART state ready to be able to start again the process */
    UartGPS.State = HAL_UART_STATE_READY;
		
		return;
  }
	
	// check data
	if(HAL_IS_BIT_SET(UartGPS.Instance->ISR, UART_FLAG_RXNE))
	{
		char c = UartGPS.Instance->RDR;
		
		// command first symbol
		if (c == '$')
		{
			gps_message_index = 0;
		}
		
		gps_tmp_string[gps_message_index] = c;
				
		if (gps_message_index > 0)
		{
			if ( gps_tmp_string[gps_message_index - 1] 	== '\r'
				&& gps_tmp_string[gps_message_index] 		  == '\n')
			{
				for (int i = 0; i < MAX_MAX_COMMAND; ++i)
				{
					if (strstr(gps_tmp_string, &gps_answers[i][0]) != 0)
					{
						//DEBUG_PRINTF("%s\r\n",gps_tmp_string);
						memset(&gps_messages[i][0], 0, GPS_MAX_MESSAGE_SIZE);
						memcpy(&gps_messages[i][0], gps_tmp_string, gps_message_index - 1);
						memset(gps_tmp_string, 0, GPS_MAX_MESSAGE_SIZE);
						
						// set true status
						messages_bit_status |= 1 << i;
					}
				}				
				
				memset(gps_tmp_string, 0, GPS_MAX_MESSAGE_SIZE);
				gps_message_index = 0;
			}
		}		
		++gps_message_index;		
		return;
	}
}

char * Get_GPS_Message(int message_type)
{
	if (messages_bit_status & (1 << message_type))
	{
		// clear bit
		//messages_bit_status &= ~(messages_bit_status | (1 << message_type));
		// return pointer to the message
		return &gps_messages[message_type][0];				
	}
	return nullptr;
}

void Reset_Message_Status(int message_type)
{
	// clear bit
	messages_bit_status &= ~(messages_bit_status | (1 << message_type));
}

