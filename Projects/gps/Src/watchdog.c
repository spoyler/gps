// Header:		wwdg.h
// File Name: wwdg.c
// Author:		andrey sidorov
// Date:			09.10.2016

#include "watchdog.h"
#include "main.h"

//WWDG_HandleTypeDef   WatchdogHandle;
WWDG_HandleTypeDef   WatchdogHandle;
extern uint32_t wdt_time;

FLASH_OBProgramInitTypeDef FlashHandle;
TIM_HandleTypeDef    TimHandle;
/* Prescaler declaration */
uint32_t uwPrescalerValue = 0;
void Timer_For_Main_Loop_Init();

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
	//
	Timer_For_Main_Loop_Init();
}

void Timer_For_Main_Loop_Init()
{
	__TIM6_CLK_ENABLE();
  /* Compute the prescaler value to have TIMx counter clock equal to 1 KHz */
  uwPrescalerValue = (uint32_t) ((SystemCoreClock / 1000) - 1);
  
  /*##-1- Configure the TIM peripheral #######################################*/ 
  /* Set TIMx instance */
  TimHandle.Instance = TIM6;
    
  /* Initialize TIMx peripheral as follow:
       + Period = 10000 - 1
       + Prescaler = SystemCoreClock/10000 Note that APB clock = TIMx clock if
                     APB prescaler = 1.
       + ClockDivision = 0
       + Counter direction = Up
  */
  TimHandle.Init.Period = 10000 - 1;
  TimHandle.Init.Prescaler = uwPrescalerValue;
  TimHandle.Init.ClockDivision = 0;
  TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  HAL_TIM_Base_Init(&TimHandle);

  /*##-2- Start the TIM Base generation in interrupt mode ####################*/
  /* Start Channel1 */
  HAL_TIM_Base_Start_IT(&TimHandle);
	HAL_NVIC_EnableIRQ(TIM6_IRQn);
}

void TIM6_DAC_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TimHandle);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	CheckMainLoopTime();
}

void Watchdog_Refresh(void)
{
	HAL_WWDG_Refresh(&WatchdogHandle, 0x7F);
}

