// Header:		
// File Name: wwdg.h
// Author:		andrey sidorov
// Date:			09.10.2016

#ifndef __WWDG_H
#define __WWDG_H

#include "stm32l0xx_hal.h"

void Watchdog_Init();
void Watchdog_Refresh(void);

#endif