// Header:
// File Name: adc.h
// Author:		andrey sidorov
// Date:			07.03.2016

#ifndef _ADC_H_
#define _ADC_H_

#include "stm32l0xx_hal.h"

void ADC_Init(void);
void ADC_Task(void);
int32_t * Get_ADC_Data(void);
void ADC_Debug();


#endif
