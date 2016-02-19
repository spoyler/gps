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
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
//#include "stm32l0xx_hal_uart.h"
#include "sim900.h"
#include <GPRS_Shield_Arduino.h>


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
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
	

const char host_name[] = "paulfertser.info";
const int host_port = 10123;


const char vtuin[] = "$VTUIN,001";
const char vtag[] = "$VTAG,001";
	
uint8_t rx_buffer[RX_BUFFER_SIZE];
UART_HandleTypeDef LUart;				// debug uart
UART_HandleTypeDef UartGSM;			// gsm uart
UART_HandleTypeDef UartGPS;			// gps uart


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

	
	__LPUART1_CLK_ENABLE();
	
	LUart.Instance = LPUART1;
	LUart.Init.BaudRate   = 115200;
	LUart.Init.WordLength = UART_WORDLENGTH_8B;
	LUart.Init.StopBits   = UART_STOPBITS_1;
	LUart.Init.Parity     = UART_PARITY_NONE;
	LUart.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	LUart.Init.Mode       = UART_MODE_TX_RX;
	LUart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
	if(HAL_UART_DeInit(&LUart) != HAL_OK)
  {
    Error_Handler(0);
  }  
  if(HAL_UART_Init(&LUart) != HAL_OK)
  {
    Error_Handler(0);
  }
		
	DEBUG_PRINTF("\r\n\r\n\r\nSystem startup!\r\n");

	// Init GSM
	gsm(&UartGSM);//RX,TX,PWR,BaudRate
	
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
	GPIO_InitStruct.Pin = /*GPIO_PIN_0 | GPIO_PIN_1 
											| */GPIO_PIN_2 | GPIO_PIN_3;
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
	
	// GPS_3dFIX high - active
//	GPIO_InitStruct.Pin = GPIO_PIN_9;
//  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	
//	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	  
	
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


PUTCHAR_PROTOTYPE
{	
	while(HAL_IS_BIT_CLR(LUart.Instance->ISR, UART_FLAG_TXE));
	LUart.Instance->TDR = ch;
  return ch;
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
