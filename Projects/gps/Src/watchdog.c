// Header:		wwdg.h
// File Name: wwdg.c
// Author:		andrey sidorov
// Date:			09.10.2016

#include "watchdog.h"

WWDG_HandleTypeDef   WatchdogHandle;
extern uint32_t wdt_time;

void Watchdog_Init()
{  
	__WWDG_CLK_ENABLE();
  /*##-2- Configure the WWDG peripheral ######################################*/
  /* WWDG clock counter = (PCLK1 (32MHz)/4096)/8) = 976.6 Hz (~1024 us) 
     WWDG Window value = 80 means that the WWDG counter should be refreshed only 
     when the counter is below 80 (and greater than 64) otherwise a reset will 
     be generated. 
     WWDG Counter value = 127, WWDG timeout = ~1024 us * 64 = 65.536 ms 
     In this case the refresh window is: ~1024 * (127-80) = 48.128 ms < refresh window < ~1024 * 64 = 65.536 ms */
  WatchdogHandle.Instance = WWDG;
  WatchdogHandle.Init.Prescaler = WWDG_PRESCALER_8;
  WatchdogHandle.Init.Window = 0x7F;
  WatchdogHandle.Init.Counter = 0x7F;

  HAL_WWDG_Init(&WatchdogHandle);


  /*##-3- Start the WWDG #####################################################*/ 
  HAL_WWDG_Start(&WatchdogHandle);
}

void Watchdog_Refresh(void)
{
	if ((HAL_GetTick() - wdt_time) < 60000)
		HAL_WWDG_Refresh(&WatchdogHandle, 127);
}
