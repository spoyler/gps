/**
  ******************************************************************************
  * @file    Multi/Examples/MotionControl/IHM04A1_ExampleFor4UniDirMotors/Src/main.c 
  * @author  IPC Rennes
  * @version V1.0.0
  * @date    January 06, 2015
  * @brief   This example shows how to use 1 IHM04A1 expansion board with 
  * 4 unidirectionnal Brush DC motors.
  * Each motor has one lead connected to one of the bridge output, 
  * the other lead to the ground. The input bridges are not parallelised.
  * The demo sequence starts when the user button is pressed.
  * Each time, the user button is pressed, the demo step is changed
  ******************************************************************************
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "sim900.h"
#include <GPRS_Shield_Arduino.h>
#include "debug.h"
#include "rtc.h"
#include "gps.h"
#include "accelero.h"
#include "adc.h"
#include "command.h"
#include "watchdog.h"

const char main_loop_timeout = 60; // sec
uint32_t main_loop_time = 0;

/** @defgroup IHM04A1_Example_for_4_Unidirectionnal_motors
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void GPIO_Init(void);
uint32_t wdt_time = 0;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	
	Init_All();
	while(1)
	{
		main_loop_time = HAL_GetTick();
		if (!Get_Debug_State(DBG))
		{
			Accelero_Task();
			RTC_Task();
			ADC_Task();
			GSM_Task();
			Command_Task();
		}
		else
		{
			Debug_Task();
		}
	}
}
void CheckMainLoopTime(void)
{
	if ((main_loop_time != 0)
		&&(abs(HAL_GetTick() - main_loop_time) > main_loop_timeout*1000))
	{
		// disable systick interrupt
		HAL_NVIC_DisableIRQ(SysTick_IRQn);
		//
		while(1);
	}
}

void Init_All()
{
	  /* STM32xx HAL library initialization */
  HAL_Init();
	/* Configure the system clock */
  SystemClock_Config();
	
  /* Set Systick Interrupt to the highest priority to have HAL_Delay working*/
  /* under the user button handler */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0x0, 0x0);    
	//
	Watchdog_Init();
	//
	GPIO_Init();
	//
	RTC_Init();
	//
	InitDebugUart();
	//
	ACCELERO_Init();
	//
	ADC_Init();
	//
	GPS_Init();
	// Init GSM
	GSM_Init();
	//	
	Command_Init();
	
	// Ckeck debug mode
	Check_Debug_Mode();
}

void ReInitPeriph()
{
	SystemClock_Config();   
	//
	InitDebugUart();
	DEBUG_PRINTF("\r\nReinit Periph!\r\n");
	DEBUG_PRINTF("Wake up from sleep mode\r\n");
	//
	ACCELERO_Init();
	//
	ADC_Init();
	//
	GPS_Init();
	// Init GSM
	GSM_Init();
}


void GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	
	
	// UART 1 aclereo
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF0_USART1;
	
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	
	//UART2  gprs
	GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);	
	
		//UART3 user
	GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_LPUART1;

	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	
  		
	// GPS_NRESET low - active
	GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
	
	// GPS_EINT1 sleep mode
	GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);	
		
	//ADC
	// control output
	GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	// adc channels
	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);	
	

	
	////////////////////////////////////////////////
	//GSM
	GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);//  digitalWrite(pin,HIGH);
	
	// Alarm port pin
	GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);	
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
	
	// Lamp port pin
	GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
	
	
	
	 /* Configure GPIO PINs to detect Interrupts */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  /* Enable and set EXTI Interrupt priority */
  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0x00, 0x00);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  error number of the error
  * @retval None
  */
void Error_Handler(uint16_t error)
{
  
  /* Infinite loop */
  while(1)
  {
  }
}




#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
