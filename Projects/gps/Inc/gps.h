// Header:
// File Name: gps.h
// Author:		andrey sidorov
// Date:			28.02.2016
#ifndef _GPS_H_
#define _GPS_H_

#include "stm32l0xx_hal.h"

void GPS_Init(void);
char GPS_Calc_CRC(char * msg, int msg_size);
void GPS_Send_Message(char * msg);
void Set_Output_Msg(void);
char * Get_GPS_Message(int message_type);
void Reset_Message_Status(int message_type);
void SetGPSSleepMode(void);

#define nullptr 0

enum {
	GGA = 0,
	ZDA = 1,
	PMTK001 = 2,
	RMC = 3
};


#endif
