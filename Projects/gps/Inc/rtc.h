// Header:
// File Name: rtc.h
// Author: andrey sidorov
// Date:	28.02.2016

#ifndef _RTC_H_
#define _RTC_H_

#include "stm32l0xx_hal.h"


#define DISPLAY_ON_DUBUGGER

#define RTC_CLOCK_SOURCE_LSI
#define RTC_ASYNCH_PREDIV  0x7F
#define RTC_SYNCH_PREDIV   0x0130

#define LSI_TIMEOUT_VALUE		100

void RTC_Init(void);
void RTC_CalendarShow(void);
void RTC_CalendarSet(char * gps_msg);
uint32_t calendar_coder(void);
void RTC_Task();
void RTC_AlarmConfig(uint32_t minute, uint32_t alarm_type);
void RTC_AlarmStop(void);
uint8_t Get_Alarm_State();
uint8_t LoadAcceleroStateFromBKP();
void SaveAcceleroStateInBKP(uint8_t state);


#endif
