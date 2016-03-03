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


/** @defgroup IHM04A1_Example_for_4_Unidirectionnal_motors
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void ButtonHandler(void);
void GPIO_Init(void);
void LedBlink(int8_t);
/* Private functions ---------------------------------------------------------*/

uint16_t gLastError = 0;
uint16_t gButtonPressed = 0;


#define BAUDRATE  9600
	

const char host_name[] = "paulfertser.info";
const int host_port = 10123;


const char vtuin[] = "$VTUIN,001";
const char vtag[] = "$VTAG,001";


UART_HandleTypeDef UartGSM;			// gsm uart
static void *LSM6DS3_X_0_handle = NULL;



char data[512];

volatile uint8_t mems_int1_detected = 0;
volatile uint8_t mems_int2_detected = 0;
volatile uint8_t state = 0;


void sendOrientation( void );


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* STM32xx HAL library initialization */
  HAL_Init();
  
  /* Configure the system clock */
  SystemClock_Config();
  
    /* Configure KEY Button */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);   
  
  /* Set Systick Interrupt to the highest priority to have HAL_Delay working*/
  /* under the user button handler */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0x0, 0x0); 
  
	GPIO_Init();
				
		
	InitDebugUart();
	DEBUG_PRINTF("\r\n\r\n\r\nSystem startup!\r\n");
	
	RTC_Init();
	
	
	GPS_Init();

	// Init GSM
	gsm(&UartGSM);//RX,TX,PWR,BaudRate
	
	//debug_simm800(&UartGSM);
	
	BSP_ACCELERO_Init( LSM6DS3_X_0, &LSM6DS3_X_0_handle );
	BSP_ACCELERO_Sensor_Enable( LSM6DS3_X_0_handle );
	
	BSP_ACCELERO_Enable_Free_Fall_Detection_Ext( LSM6DS3_X_0_handle );
	
	
	LSM6DS3_ACC_GYRO_W_INACTIVITY_ON(&LSM6DS3_X_0_handle, LSM6DS3_ACC_GYRO_INACTIVITY_ON_ENABLED);
	LSM6DS3_ACC_GYRO_W_SLEEP_DUR(LSM6DS3_X_0_handle, 7);
	LSM6DS3_ACC_GYRO_W_WAKE_DUR(LSM6DS3_X_0_handle, 2);
	LSM6DS3_ACC_GYRO_W_WK_THS(LSM6DS3_X_0_handle, 2);
	
	LSM6DS3_ACC_GYRO_W_SleepEvOnInt1(LSM6DS3_X_0_handle, LSM6DS3_ACC_GYRO_INT1_SLEEP_ENABLED);
	
  /* Infinite loop */	
	uint8_t  status = 0;
	
	char * ptr_gps_msg = 0;
	
/*	while(1)
	{
		RTC_CalendarShow();
		HAL_Delay(10000);
	}
*/
	while(1)
	{
		// check interrupt from gyroscope
		if ( mems_int2_detected != 0 )
    {
      if ( BSP_ACCELERO_Get_Free_Fall_Detection_Status_Ext( LSM6DS3_X_0_handle, &status ) == COMPONENT_OK )
      {
        if ( status != 0 )
        {
					DEBUG_PRINTF("Free Fall Detection!!!!\r\n");
        }
      }
      mems_int2_detected = 0;
    }
		if (mems_int1_detected)
		{
			mems_int1_detected = 0;
			state = !state;
			DEBUG_PRINTF("Active state = %d\r\n", state);
		}


		// if state active then send data to the server
		if (state)
		{
			if (!is_connected())
			{
				DEBUG_PRINTF("Connecting to %s:%d... ", host_name, host_port);
				if (connect(TCP,host_name, host_port, 2, 100))
				{
					DEBUG_PRINTF("Ok\r\n");
				}
				else
				{
					DEBUG_PRINTF("Error\r\n");
				}
			}
			else
			{
				ptr_gps_msg = (char *)Get_GPS_Message(GGA);
				
				if (ptr_gps_msg != nullptr)
				{
					memset(data,0, 512);					
					sprintf(data, "%s\r\n%s\r\n%s\r\n", vtuin, ptr_gps_msg, vtag);				
					//sprintf(data, "%s\r\n", ptr_gps_msg);				
					send(data, strlen(data));
					DEBUG_PRINTF("%s", data);
				}
			}			
		}
		
		// time synchro
		ptr_gps_msg = (char *)Get_GPS_Message(ZDA);
		if (ptr_gps_msg != nullptr)
		{
			RTC_CalendarSet(ptr_gps_msg);
		}		
	}
}

void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
	if ( GPIO_Pin == M_INT1_PIN )
  {
    mems_int1_detected = 1;
  }
	else
		if (GPIO_Pin == M_INT2_PIN)
		{
			mems_int2_detected = 1;
		}
}


void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UartGSM);
}

void GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	
	
	// UART 1
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF0_USART1;
	
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	
	//UART2
	GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);	
	
		//UART3
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

	
	////////////////////////////////////////////////
	//GSM
	GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	

}

/**
  * @brief  This function is the User handler for the flag interrupt
  * @param  None
  * @retval None
  */
void MyFlagInterruptHandler(void)
{
  
 }

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  error number of the error
  * @retval None
  */
void Error_Handler(uint16_t error)
{
  /* Backup error number */
  gLastError = error;
  
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
