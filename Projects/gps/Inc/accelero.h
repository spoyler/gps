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

void ACCELERO_Init();
uint8_t Get_Accelero_State();
int Accelero_Task();
SensorAxesRaw_t * Get_ACC_Data();
SensorAxesRaw_t * Get_GYRO_Data();


#endif