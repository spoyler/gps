// Header:
// File Name: accelero.h
// Author:		andrey sidorov
// Date:			07.03.2016


#ifndef _ACCELERO_H_
#define _ACCELERO_H_

#include "stm32l0xx_hal.h"
#include "x_nucleo_iks01a1.h"
#include "x_nucleo_iks01a1_accelero.h"
#include "x_nucleo_iks01a1_gyro.h"

enum
{
	MODE_OFF = 0,
	MODE_ON = 1
};

void ACCELERO_Init();
uint8_t Get_Accelero_State();
uint8_t Get_Free_Fall_State();
void Accelero_Task();
SensorAxesRaw_t * Get_ACC_Data();
SensorAxesRaw_t * Get_GYRO_Data();
void Set_Gyro_Sleep_Mode(uint8_t new_mode);

void EnableWakeupDetection(void);
void DisableWakeupDetection(void);
void ACC_Debug(void);


#endif