// Header:		rtc.h
// File Name: rtc.c
// Author:		andrey sidorov
// Date:			28.02.2016

#include <stdio.h>
#include <stdlib.h>
#include "rtc.h"
#include "debug.h"
#include "gps.h"

uint32_t last_time_tick = 0;
/* RTC handler declaration */
RTC_HandleTypeDef RtcHandle; 
TIM_HandleTypeDef Input_Handle;
/* Buffers used for displaying Time and Date */
uint8_t aShowTime[50] = {0}, aShowTimeStamp[50] = {0};
uint8_t aShowDate[50] = {0}, aShowDateStamp[50] = {0};

uint16_t tmpCC4[2] = {0, 0};
__IO uint32_t uwLsiFreq = 0;

__IO uint32_t uwCaptureNumber = 0;
__IO uint32_t uwPeriodValue = 0; 
 
FlagStatus TamperStatus = RESET;

void RTC_TimeStampConfig(void);
void RTC_Config(void);
static uint32_t GetLSIFrequency(void);

void RTC_Init()
{	
	RTC_Config();
	
	/* Get the LSI frequency:  TIM21 is used to measure the LSI frequency */
  uwLsiFreq = GetLSIFrequency(); 
	
  /* Update the Calendar Configuration with the LSI exact value */
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = 0x7F;
  RtcHandle.Init.SynchPrediv = (uwLsiFreq/128) - 1;
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	
	HAL_RTC_Init(&RtcHandle);

    /*##-2-  Configure RTC Timestamp ############################################*/
  RTC_TimeStampConfig();
	
	last_time_tick = HAL_GetTick();
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

//  /*##-1- Backup Domain reset ################################################*/
//  __HAL_RCC_BACKUPRESET_FORCE(); 
//  __HAL_RCC_BACKUPRESET_RELEASE();

  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
  
	HAL_RCC_OscConfig(&RCC_OscInitStruct);


  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  
  /*##-3- Enable RTC peripheral Clocks #######################################*/ 
  /* Enable RTC Clock */ 
  __HAL_RCC_RTC_ENABLE(); 
  
  /*##-4- Configure the NVIC for RTC Tamper ###################################*/
  HAL_NVIC_SetPriority(RTC_IRQn, 0x0, 0);
  HAL_NVIC_EnableIRQ(RTC_IRQn);
}

/**
  * @brief RTC MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  * @param hrtc: RTC handle pointer
  * @retval None
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
  /*##-1- Reset peripherals ##################################################*/
   __HAL_RCC_RTC_DISABLE();
} 

void RTC_Config(void)
{
	
	uint32_t tickstart = 0;
	 /* Enable the Internal Low Speed oscillator (LSI). */
	__HAL_RCC_LSI_ENABLE();
	
	/* Get timeout */
	tickstart = HAL_GetTick();
	
	/* Wait till LSI is ready */  
	while(__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) == RESET)
	{
		if((HAL_GetTick() - tickstart ) > LSI_TIMEOUT_VALUE)
		{
			return;
		}      
  } 
  /*##-1- Configure the RTC peripheral #######################################*/
  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follow:
      - Hour Format    = Format 24
      - Asynch Prediv  = Value according to source clock
      - Synch Prediv   = Value according to source clock
      - OutPut         = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType     = Open Drain */
  RtcHandle.Instance = RTC; 
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV; /* 0x7F */
  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV; /* (39KHz / 128) - 1 = 0x130 */
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  
  HAL_RTC_Init(&RtcHandle);
  
  /*##-2- Configure the RTC Wake up peripheral #################################*/
  /* Setting the Wake up time to 1 s
       If RTC_WAKEUPCLOCK_CK_SPRE_16BITS is selected, the frequency is 1Hz, 
       this allows to get a wakeup time equal to 1 s if the ciunter is 0x0 */
  //HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, 0x0, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
}

void RTC_TimeStampConfig(void)
{
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;
  
  /*##-3- Configure the Date #################################################*/
  /* Set Date: Monday April 14th 2014 */
  sdatestructure.Year = 0x14;
  sdatestructure.Month = RTC_MONTH_APRIL;
  sdatestructure.Date = 0x14;
  sdatestructure.WeekDay = RTC_WEEKDAY_MONDAY;
  
  HAL_RTC_SetDate(&RtcHandle,&sdatestructure,RTC_FORMAT_BCD);
  
  /*##-4- Configure the Time #################################################*/
  /* Set Time: 08:10:00 */
  stimestructure.Hours = 0x08;
  stimestructure.Minutes = 0x10;
  stimestructure.Seconds = 0x00;
  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;
  
  HAL_RTC_SetTime(&RtcHandle,&stimestructure,RTC_FORMAT_BCD);
}

/**
  * @brief  Configures TIM21 to measure the LSI oscillator frequency. 
  * @param  None
  * @retval LSI Frequency
  */
static uint32_t GetLSIFrequency(void)
{
  TIM_IC_InitTypeDef    TIMInput_Config;

  /* Configure the TIM peripheral *********************************************/ 
  /* Set TIMx instance */  
	
	__TIM21_CLK_ENABLE();
	
  Input_Handle.Instance = TIM21;
  
  /* TIM21 configuration: Input Capture mode ---------------------
     The LSI oscillator is connected to TIM21 CH1.
     The Rising edge is used as active edge.
     The TIM21 CCR1 is used to compute the frequency value. 
  ------------------------------------------------------------ */
  Input_Handle.Init.Prescaler         = 0; 
  Input_Handle.Init.CounterMode       = TIM_COUNTERMODE_UP;  
  Input_Handle.Init.Period            = 0xFFFF; 
  Input_Handle.Init.ClockDivision     = 0;     
  if(HAL_TIM_IC_Init(&Input_Handle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
  /* Connect internally the TIM21_CH1 Input Capture to the LSI clock output */
  HAL_TIMEx_RemapConfig(&Input_Handle, TIM21_TI1_LSI);
  
  /* Configure the Input Capture of channel 1 */
  TIMInput_Config.ICPolarity  = TIM_ICPOLARITY_RISING;
  TIMInput_Config.ICSelection = TIM_ICSELECTION_DIRECTTI;
  TIMInput_Config.ICPrescaler = TIM_ICPSC_DIV8;
  TIMInput_Config.ICFilter    = 0;
	
	HAL_NVIC_EnableIRQ(TIM21_IRQn);
	
  if(HAL_TIM_IC_ConfigChannel(&Input_Handle, &TIMInput_Config, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Start the TIM Input Capture measurement in interrupt mode */
  if(HAL_TIM_IC_Start_IT(&Input_Handle, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  /* Wait until the TIM21 get 2 LSI edges */
  while(uwCaptureNumber != 2)
  {
  }

  /* Disable TIM21 CC1 Interrupt Request */
  HAL_TIM_IC_Stop_IT(&Input_Handle, TIM_CHANNEL_1);
  
  /* Deinitialize the TIM21 peripheral registers to their default reset values */
  HAL_TIM_IC_DeInit(&Input_Handle);

  return uwLsiFreq;
} 

/**
  * @brief  This function handles TIM2 interrupt request.
  * @param  None
  * @retval None
  */
void TIM21_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&Input_Handle);
}

/**
  * @brief  Input Capture callback in non blocking mode 
  * @param  htim : TIM IC handle
  * @retval None
*/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  /* Get the Input Capture value */
  tmpCC4[uwCaptureNumber++] = HAL_TIM_ReadCapturedValue(&Input_Handle, TIM_CHANNEL_1);
  
  if (uwCaptureNumber >= 2)
  {
    if ( tmpCC4[0] > tmpCC4[1] )
    {
      /* Compute the period length */
      uwPeriodValue = (uint16_t)(0xFFFF - tmpCC4[0] + tmpCC4[1] + 1);
    }
    else
    {
      /* Compute the period length */
      uwPeriodValue = (uint16_t)(tmpCC4[1] - tmpCC4[0]);
    }
    /* Frequency computation */ 
    uwLsiFreq = (uint32_t) SystemCoreClock / uwPeriodValue;
    uwLsiFreq *= 8;
  }
} 

/**
  * @brief  Display the current time and date.
  * @param  showtime : pointer to buffer
  * @param  showdate : pointer to buffer
  * @retval None
  */
void RTC_CalendarShow(void)
{
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;
  
  /* Get the RTC current Time */
  HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
  
  /* Display time Format : hh:mm:ss */
  DEBUG_PRINTF("%.2d:%.2d:%.2d\r\n", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
  /* Display date Format : mm-dd-yy */
  DEBUG_PRINTF("%.2d-%.2d-%.2d\r\n", sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);
} 


void RTC_CalendarSet(char * gps_msg)
{
	//gps_msg format // $GNZDA,201705.089,29,02,2016,,*44
	
  RTC_DateTypeDef new_date = {0};
  RTC_TimeTypeDef new_time = {0};
	
	int32_t year = 0;
	char date_time[5] = {0};
	
	// Hours
	date_time[0] = gps_msg[7]; date_time[1] = gps_msg[8];
	new_time.Hours = atoi(date_time);
	// Minutes
	date_time[0] = gps_msg[9]; date_time[1] = gps_msg[10];
	new_time.Minutes = atoi(date_time);
	// Seconds
	date_time[0] = gps_msg[11]; date_time[1] = gps_msg[12];
	new_time.Seconds = atoi(date_time);
	
	// Date
	date_time[0] = gps_msg[18]; date_time[1] = gps_msg[19];
	new_date.Date = atoi(date_time);
	
	// Month
	date_time[0] = gps_msg[21]; date_time[1] = gps_msg[22];
	new_date.Month = atoi(date_time);
	
	// Year
	date_time[0] = gps_msg[24]; date_time[1] = gps_msg[25];
	date_time[2] = gps_msg[26]; date_time[3] = gps_msg[27];
	new_date.Year = (year = atoi(date_time) - 2000) >= 0 ? year : 0;
	
	if ((new_time.Seconds >= 0 &&  new_time.Seconds <= 59) &&
		  (new_time.Minutes >= 0 &&  new_time.Minutes <= 59) &&
		  (new_time.Hours >= 0 &&  new_time.Hours <= 23)		 &&
			(new_date.Date >= 1 &&  new_date.Date <= 31)		 	 &&
			(new_date.Month >= 1 &&  new_date.Month <= 12)		 &&
			(new_date.Year > 0 &&  new_date.Year < 79))
	{
		
		new_time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		new_time.StoreOperation = RTC_STOREOPERATION_SET;
		
		/* Display time Format : hh:mm:ss */
//		DEBUG_PRINTF("%.2d:%.2d:%.2d\r\n", new_time.Hours, new_time.Minutes, new_time.Seconds);
		/* Display date Format : mm-dd-yy */
//		DEBUG_PRINTF("%.2d-%.2d-%.2d\r\n", new_date.Month, new_date.Date, 2000 + new_date.Year);
		
		/* Get the RTC current Date */
		HAL_RTC_SetDate(&RtcHandle, &new_date, RTC_FORMAT_BIN);
		/* Get the RTC current Time */
		HAL_RTC_SetTime(&RtcHandle, &new_time, RTC_FORMAT_BIN);

		
		//
//		RTC_CalendarShow();
	}
}

void RTC_Task()
{
	// time synchro
	char * ptr_gps_msg = (char *)Get_GPS_Message(ZDA);
	if (ptr_gps_msg != nullptr)
	{
		RTC_CalendarSet(ptr_gps_msg);
	}	
	
	uint32_t tick = HAL_GetTick();
	if ((tick - last_time_tick) > 10000)
	{
		last_time_tick = tick;
		RTC_CalendarShow();
	}
}


































