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


/** @defgroup IHM04A1_Example_for_4_Unidirectionnal_motors
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RX_BUFFER_SIZE 256

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
	
uint8_t rx_buffer[RX_BUFFER_SIZE];
UART_HandleTypeDef UartGSM;			// gsm uart
UART_HandleTypeDef UartGPS;			// gps uart
static void *LSM6DS3_X_0_handle = NULL;


char gps_message[256];
char gps_tmp_string[256];
int gps_message_index = 0;
int gps_message_recived = 0;

char data[512];

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
				
	__USART1_CLK_ENABLE();
  UartGPS.Instance        = USART1;

  UartGPS.Init.BaudRate   = 9600;
  UartGPS.Init.WordLength = UART_WORDLENGTH_8B;
  UartGPS.Init.StopBits   = UART_STOPBITS_1;
  UartGPS.Init.Parity     = UART_PARITY_NONE;
  UartGPS.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  UartGPS.Init.Mode       = UART_MODE_TX_RX;
  UartGPS.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if(HAL_UART_DeInit(&UartGPS) != HAL_OK)
  {
    Error_Handler(0);
  }  
  if(HAL_UART_Init(&UartGPS) != HAL_OK)
  {
    Error_Handler(0);
  }
	
		__HAL_UART_ENABLE_IT(&UartGPS, UART_IT_ORE | UART_IT_RXNE);
	
	/* Process Unlocked */
	__HAL_UNLOCK(&UartGPS); 
	HAL_NVIC_EnableIRQ(USART1_IRQn);
		
	DEBUG_PRINTF("\r\n\r\n\r\nSystem startup!\r\n");

	// Init GSM
	gsm(&UartGSM);//RX,TX,PWR,BaudRate
	
	BSP_ACCELERO_Init( LSM6DS3_X_0, &LSM6DS3_X_0_handle );
	
  /* Infinite loop */
	int count = 0;
  while(1)
  {			
		if (gps_message_recived)
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
				memset(data,0, 512);					
				sprintf(data, "%s\r\n%s\r\n%s\r\n", vtuin, gps_message, vtag);				
				send(data, strlen(data));
				DEBUG_PRINTF("%s", data);
				gps_message_recived = 0;			
			}
		}
  }
}

void USART1_IRQHandler(void)
{
	
	// check errors
	if(__HAL_UART_GET_IT(&UartGPS, UART_IT_ORE) != RESET) 
  { 
    __HAL_UART_CLEAR_IT(&UartGPS, UART_CLEAR_OREF);
    
    UartGPS.ErrorCode |= HAL_UART_ERROR_ORE;
    /* Set the UART state ready to be able to start again the process */
    UartGPS.State = HAL_UART_STATE_READY;
		
		return;
  }
	
	// check data
	if(HAL_IS_BIT_SET(UartGPS.Instance->ISR, UART_FLAG_RXNE))
	{
		char c = UartGPS.Instance->RDR;
		
		if (c == '$')
		{
			gps_message_index = 0;
		}
		
		gps_tmp_string[gps_message_index] = c;
				
		if (gps_message_index > 0)
		{
			if (   gps_tmp_string[gps_message_index - 1] 	== '\r'
					&& gps_tmp_string[gps_message_index] 			== '\n')
			{
				if (/*(strstr(gps_tmp_string, "GGA") != 0)
					&&*/ !gps_message_recived )
				{
					memset(gps_message, 0, 256);
					memcpy(gps_message, gps_tmp_string, gps_message_index - 1);
					memset(gps_tmp_string, 0, 256);
					gps_message_recived = 1;
				}
				memset(gps_tmp_string, 0, 256);
				gps_message_index = 0;
			}
		}		
		++gps_message_index;
		
		return;
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

	//SPI Pin Init
	GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_14 |GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
	
	// SPI CS
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	
	GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);	
	
	 /* Configure GPIO PINs to detect Interrupts */
  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* Enable and set EXTI Interrupt priority */
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0x00, 0x00);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
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

/**
  * @brief  This function is executed when the Nucleo User button is pressed
  * @param  error number of the error
  * @retval None
  */
void ButtonHandler(void)
{
  gButtonPressed = 1;
  
  /* Let 300 ms before clearing the IT for key debouncing */
  HAL_Delay(300);
  __HAL_GPIO_EXTI_CLEAR_IT(KEY_BUTTON_PIN);
  HAL_NVIC_ClearPendingIRQ(KEY_BUTTON_EXTI_IRQn);
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
