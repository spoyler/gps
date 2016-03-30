// Header:		command.h
// File Name: command.c
// Author:		andrey sidorov
// Date:			19.03.2016

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "command.h"
#include "debug.h"
#include "accelero.h"
#include "gps.h"
#include "adc.h"
#include "rtc.h"

const uint32_t buffer_size = 8;
const uint32_t buffer_lenght = 256;
const uint32_t buffer_lenght_answ = 32;

uint16_t ServerCommands[8] = {0};

typedef struct
{
	uint32_t head;
	uint32_t tail;
	uint32_t buffer_pos;
	uint8_t data[buffer_size][buffer_lenght];
}SBuffer;

typedef struct
{
	uint32_t head;
	uint32_t tail;
	uint32_t buffer_pos;
	uint8_t data[buffer_size][buffer_lenght_answ];
}SAnswers;

enum
{
	CLEAR = 0,
	NO_ACTION = 1,
	CMD_END = 2,
	BUFFER_END = 3
};

SBuffer buffer = {0};
SAnswers answers = {0};

const char tmark[] = "$TMARK";
const char uin	[] = "$UIN";
const char gpgga[] = "$GPGGA";
const char gprmc[] = "$GPRMC";
const char ag		[] = "$AG";
const char event[] = "$EVENT";
const char volt	[] = "$VOLT";
const char eof	[] = "$EOF";
const char cmd	[] = "$CMD";
const char ack	[] = "$ACK,0,0\r\n";

enum
{
	TMARK = 0,
	UIN		= 1,
	GPGGA = 2,
	GPRMC = 3,
	AG		= 4,
	EVENT = 5,
	VOLT	= 6,
	EOC		= 7,
	CDM 	= 8,
	ACK		= 9
};

const char cmd_items = 10;

SSystemInfo system_info = {0};

uint32_t packet_number = 0;
uint32_t last_tick = 0;


void Command_Init()
{
	system_info.tracker_id = 1;
	system_info.sw_version = 1;
	system_info.hw_version = 1;
	
	
	// set period of data send 10 sec
	
	ServerCommands[1] = 10;
	
	last_tick = HAL_GetTick();
}

uint32_t PushToBuffer(uint32_t param, const char * string, ...  )
{
	if (param == CLEAR)
	{
		buffer.buffer_pos = 0;
		memset(&buffer.data[buffer.head],0, buffer_lenght);
	}
	
	uint32_t free_space = buffer_lenght - buffer.buffer_pos;
	{
		va_list args;
    va_start (args, string);
		buffer.buffer_pos += vsnprintf(&buffer.data[buffer.head][buffer.buffer_pos], free_space, string,  args);
		va_end(args);
		
		if (param == CMD_END)
		{
			buffer.data[buffer.head][buffer.buffer_pos++] = 0x0d;
			buffer.data[buffer.head][buffer.buffer_pos++] = 0x0a;
		}
		
		return 0;
	}
}

void EndBufferWrite()
{
	buffer.head = (buffer.head + 1) & (buffer_size - 1);
	
	if (buffer.head == buffer.tail)
	{
			buffer.tail = (buffer.tail + 1) & (buffer_size - 1);
	}
}

uint32_t ReadBuffer(uint8_t ** pointer)
{
	if (buffer.head != buffer.tail)
	{
		*pointer = (uint8_t*)buffer.data[buffer.tail];
		buffer.tail = (buffer.tail + 1) & (buffer_size - 1);
		
		return buffer.buffer_pos;
	}	
	else
	{
		*pointer = (uint8_t*)NULL;
		return 0;
	}
}


uint32_t PushToAnswers(void * data, uint32_t size, uint32_t param)
{
	if (param == CLEAR)
	{
		answers.buffer_pos = 0;
		memset(&answers.data[answers.head],0, buffer_lenght_answ);
	}
	
	if ((buffer_lenght_answ - answers.buffer_pos) >= size)
	{
		memcpy(&answers.data[answers.head][answers.buffer_pos], data, size);
		answers.buffer_pos += size;
		
		if (param == CMD_END)
		{
			answers.data[answers.head][answers.buffer_pos++] = 0x0d;
			answers.data[answers.head][answers.buffer_pos++] = 0x0a;
		}
		
		return 0;
	}
	else
	{
		return -1;
	}
}

void EndBufferWriteAnsw()
{
	answers.head = (answers.head + 1) & (buffer_size - 1);
	
	if (answers.head == answers.tail)
	{
			answers.tail = (answers.tail + 1) & (buffer_size - 1);
	}
}

uint32_t ReadBufferAnsw(uint8_t ** pointer)
{
	if (answers.head != answers.tail)
	{
		*pointer = (uint8_t*)answers.data[answers.tail];
		answers.tail = (answers.tail + 1) & (buffer_size - 1);
		return answers.buffer_pos;
	}	
	else
	{
		*pointer = (uint8_t*)NULL;
		return 0;
	}
}


void Command_Task()
{
	//	
	if (Get_Accelero_State())
	{
		const char *gps_gga = (char *)Get_GPS_Message(GGA);
		const char *gps_rmc = (char *)Get_GPS_Message(RMC);
		
		uint8_t free_fall_state = Get_Free_Fall_State();
		
		if (((HAL_GetTick() - last_tick) > ServerCommands[1])
				|| free_fall_state )
		{
			last_tick =  HAL_GetTick();

			if ((gps_gga != nullptr) && (gps_rmc != nullptr))
			{
				Reset_Message_Status(GGA);
				Reset_Message_Status(RMC);
				
				int size = 0;
				
				SensorAxesRaw_t * acc_data = Get_ACC_Data();
				SensorAxesRaw_t * gyro_data = Get_GYRO_Data();
				int32_t * adc_data = Get_ADC_Data();
				
				// set uin---------------------------------------------------------------
				PushToBuffer(CLEAR, "%s,%d,%d,%d\r\n", uin, system_info.tracker_id, system_info.sw_version, system_info.hw_version);
				
				// set tmark-------------------------------------------------------------
				// get time
				// copy seconds
				// copy packet naumber
				// copy packet naumber
				uint32_t time = calendar_coder();
				uint16_t sub_sec = 0;
				packet_number++;
				PushToBuffer(NO_ACTION, "%s,%d,%d,%d\r\n", tmark, time, sub_sec, packet_number );
				
				// set gga---------------------------------------------------------------			
				PushToBuffer(NO_ACTION, "%s\r\n", gps_gga);
				
				// set rmc---------------------------------------------------------------
				PushToBuffer(NO_ACTION, "%s\r\n", gps_rmc);
				
				// set motion state------------------------------------------------------
				PushToBuffer(NO_ACTION, "%s,%d,%d,%d,%d,%d,%d\r\n", ag, acc_data->AXIS_X, acc_data->AXIS_Y, acc_data->AXIS_X,
																					gyro_data->AXIS_X, gyro_data->AXIS_Y, gyro_data->AXIS_Z);
				
				// set event state-------------------------------------------------------
				PushToBuffer(NO_ACTION, "%s,%d,0\r\n", event, free_fall_state);
				
				// set adc result--------------------------------------------------------
				PushToBuffer(NO_ACTION, "%s,%d,%d\r\n", volt, abs(adc_data[0]), abs(adc_data[1]));
				
				// push end of file cmd -------------------------------------------------
				PushToBuffer(NO_ACTION, "%s\r\n", eof);	

				EndBufferWrite();
			}
		}
	}
	else
	{
		last_tick =  HAL_GetTick();
	}
}

int Parse_Command(char * data, int size)
{
	/* $CMD,N,1,0\r\n*/
	char * ptr = nullptr;
	int16_t reg = -1;
	int16_t value = -1;
	uint8_t error_code = 0;
	
	ptr = (char*)strstr(data, cmd);
	
	if (ptr != nullptr)
	{
		ptr += strlen(cmd) + 1;
		
		char command = *ptr;
		
		ptr += 2;
				
		// get register address
		char * end_str = (char *)strchr(ptr, ',');
		
		if (end_str != nullptr)
		{
			char number[8] = {0};		
			int size = (uint32_t)end_str - (uint32_t)(ptr);
			size = size < 8 ? size : 8;
			strncpy ( number, ptr, size );
			
			reg = atoi(number);
			
			// get command value
			
			ptr = end_str + 1;
			
			end_str = (char *)strchr(ptr, '\r');
			
			if (end_str != nullptr)
			{
				memset(number, 0, 8);
				size = (uint32_t)end_str - (uint32_t)(ptr);
				size = size < 8 ? size : 8;
				strncpy ( number, ptr, size );
				
				value = atoi(number);
				
			}
			else
			{
				error_code = 2;
			}
			
			
			if ((reg >= 0) && (reg <= 7) 
					&& (value >= 0) && (value <= 7))
			{
				switch (command)
				{
					case 'W':
						if (reg == 0)
						{
							switch (value)
							{
								case 1:
									DEBUG_PRINTF("REBOOT\r\n");
								break;
								case 2:
									DEBUG_PRINTF("SLEEP\r\n");
								break;
								case 3:
									DEBUG_PRINTF("ALARM\r\n");
								break;
								default:
									value = 0;
									error_code = 3;
									DEBUG_PRINTF("Unknown Command\r\n");
								break;
							}
						}
						else
						{
							ServerCommands[reg] = value;
						}
					break;
					case 'R':
						if (reg == 0)
						{
							value = 0;
						}
						else
						{
							value = ServerCommands[reg];
						}
					break;
					
					case 'N':						
					break;
				}				
			}
		}
		else		
		{
			error_code = 1;			
		}
	
		PushToAnswers((void *)ack, sizeof(ack)-1, CLEAR);
		
		//PushToAnswers((void *)&error_code, sizeof(uint8_t), NO_ACTION);
		//PushToAnswers((void *)&value, sizeof(uint16_t), CMD_END);		
		EndBufferWriteAnsw();
		
	}
	
	
	return -1;
}
