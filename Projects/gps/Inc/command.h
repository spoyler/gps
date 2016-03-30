// Header:		command.h
// File Name: command.h
// Author:		andrey sidorov
// Date:			19.03.2016

#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "stm32l0xx_hal.h"


typedef struct
{
	uint32_t tracker_id;
	uint8_t  hw_version;
	uint8_t  sw_version;		
}SSystemInfo;

void Command_Init();
void Command_Task();
uint32_t ReadBuffer(uint8_t ** pointer);

uint32_t PushToBuffer(uint32_t param, const char * string, ...  );
uint32_t PushToAnswers(void * data, uint32_t size, uint32_t param);
uint32_t ReadBufferAnsw(uint8_t ** pointer);
uint8_t IsDataToSend();

int Parse_Command(char * data, int size);

#endif
