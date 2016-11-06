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
	uint16_t host_port;	
	char host_name[32];	
}SSystemInfo;

enum
{
	MESSAGE_PERIOD = 0,
	WAIT_BEFOR_SLEEP = 1,
	SLEEP_TIME = 2,
	ALARM_TIME = 3,
	MAX_PARAMS = 8
};

enum
{
	EVENT_FREE_FALL = 1,
	EVENT_THRESHOLD_GYRO = 2,
	EVENT_THRESHOLD_ACC = 3,
	EVENT_SLEEP = 4,
	EVENT_WAKEUP = 5,
	MAX_EVENTS = 6	
};

enum
{
	EVENT_NONE = 0,
	EVENT_ACTIVE = 1,
	EVENT_PUSHED_TO_BUFFER = 2,
	EVENT_SEND = 3,
	ERROR_EVENT = 4
};


enum
{
	WAKEUP_SOURCE_ACC = 0,
	WAKEUP_SOURCE_TIMER = 1,
	WAKEUP_SOURCE_SERVER = 2,
	WAKEUP_SOURCE_CHARGING = 3
};



void Command_Init();
void Command_Task();


uint32_t PushToBuffer(uint32_t param, const char * string, ...  );
uint32_t ReadBuffer(uint8_t ** pointer, uint8_t* flags);
void SetBufferFlags(uint8_t flag_position);
uint32_t PushToAnswers(void * data, uint32_t size, uint32_t param);
uint32_t ReadBufferAnsw(uint8_t ** pointer);
uint8_t IsDataToSend();


int Parse_Command(char * data, int size);
uint16_t GetParamValue(uint8_t param);
void DisableExternalDevices(void);
void SetEventState(uint8_t event, uint8_t event_state);
uint8_t GetEventState(uint8_t event);
void SetWakeupSource(uint8_t source);
uint8_t GetWakeupSource(void);
uint8_t GetChargingState(void);
void ChargingOn(void);
void ChargingOFF(void);
void Command_Debug(void);



#endif
