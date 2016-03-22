// Header:		command.h
// File Name: command.c
// Author:		andrey sidorov
// Date:			19.03.2016

#include <stdio.h>
#include <string.h>

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
const char ack	[] = "$ACK";

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

uint32_t PushToBuffer(void * data, uint32_t size, uint32_t param)
{
	if (param == CLEAR)
	{
		buffer.buffer_pos = 0;
		memset(&buffer.data[buffer.head],0, buffer_lenght);
	}
	
	if ((buffer_lenght - buffer.buffer_pos) >= size)
	{
		memcpy(&buffer.data[buffer.head][buffer.buffer_pos], data, size);
		buffer.buffer_pos += size;
		
		if (param == CMD_END)
		{
			buffer.data[buffer.head][buffer.buffer_pos++] = 0x10;
			buffer.data[buffer.head][buffer.buffer_pos++] = 0x13;
		}
		
		return 0;
	}
	else
	{
		return -1;
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
			answers.data[answers.head][answers.buffer_pos++] = 0x10;
			answers.data[answers.head][answers.buffer_pos++] = 0x13;
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
				
				int32_t motion_data[6];
				
				motion_data[0] = acc_data->AXIS_X;
				motion_data[1] = acc_data->AXIS_Y;
				motion_data[2] = acc_data->AXIS_Z;
				
				motion_data[3] = gyro_data->AXIS_X;
				motion_data[4] = gyro_data->AXIS_Y;
				motion_data[5] = gyro_data->AXIS_Z;
				
				// set uin---------------------------------------------------------------
				PushToBuffer((void *)uin, sizeof(uin)-1, CLEAR);
				PushToBuffer((void *)&system_info, /*sizeof(system_info)*/ 6, CMD_END);
				
				// set tmark-------------------------------------------------------------
				PushToBuffer((void*)tmark, sizeof(tmark) - 1, NO_ACTION);
				
				// get time
				uint32_t time = calendar_coder();
				
				// copy seconds
				PushToBuffer(&time, sizeof(time), NO_ACTION);
				
				// copy subseconds
				uint16_t sub_sec = 0;
				PushToBuffer(&sub_sec, sizeof(uint16_t), NO_ACTION);
				
				// copy packet naumber
				packet_number++;
				PushToBuffer(&packet_number, sizeof(packet_number), CMD_END);						
				
				// set gga---------------------------------------------------------------			
				PushToBuffer((void *)gps_gga, strlen(gps_gga), CMD_END);
				
				// set rmc---------------------------------------------------------------
				PushToBuffer((void *)gps_rmc, strlen(gps_rmc), CMD_END);
				
				// set motion state------------------------------------------------------
				PushToBuffer((void *)ag, sizeof(ag) - 1, NO_ACTION);
				PushToBuffer((void *)motion_data, sizeof(motion_data[1])*6, CMD_END);
				
				// set event state-------------------------------------------------------
				PushToBuffer((void *)event, sizeof(event) - 1, NO_ACTION);
				PushToBuffer((void *)&free_fall_state, sizeof(uint8_t) - 1, CMD_END);
				
				// set adc result--------------------------------------------------------
				PushToBuffer((void *)volt, sizeof(volt) - 1, NO_ACTION);
				PushToBuffer((void *)adc_data, sizeof(int32_t)*2, CMD_END);
				
				// push end of file cmd -------------------------------------------------
				PushToBuffer((void *)eof, sizeof(eof) - 1, CMD_END);	
					
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
		PushToAnswers((void *)&error_code, sizeof(uint8_t), NO_ACTION);
		PushToAnswers((void *)&value, sizeof(uint16_t), CMD_END);		
		EndBufferWriteAnsw();
		
	}
	
	
	return -1;
}
