/**
  ******************************************************************************
  * @file    Multi/Examples/MotionControl/IHM04A1_ExampleFor4UniDirMotors/Src/stm32l0xx_it.c 
  * @author  IPC Rennes
  * @version V1.0.0
  * @date    January 06, 2015
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
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
#include "stm32l0xx_it.h"
#include "debug.h"
#include "watchdog.h"


    
/** @addtogroup Interrupt_Handlers
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/    
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/******************************************************************************/
/*            Cortex-M0+ Processor Exceptions Handlers                         */
/******************************************************************************/

static void Hard_fault_handler_c(unsigned int* hardfault_args);
register unsigned int R0 __asm("r0");
register uint32_t contr_reg __asm("control");
register unsigned int MSP __asm("msp");
register unsigned int PSP __asm("psp");
uint32_t *ptr;

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
	
 if(contr_reg & 2)
   {
     ptr = (uint32_t*)PSP;
   }
   else
   {
     ptr = (uint32_t*)MSP;
   }
    R0 = (unsigned int)ptr;
   __asm("BL Hard_fault_handler_c");                    //top of stack is in R0. It is passed to C-function.
	 


  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
	Watchdog_Refresh();
}

/******************************************************************************/
/*                 STM32L0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l0xx.s).                                               */
/******************************************************************************/


/**
  * @brief  This function handles interrupt for External lines 0 to 1
  * @param  None
  * @retval None
  */
void EXTI0_1_IRQHandler(void)
{

}

void EXTI2_3_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

/**
  * @brief  This function handles interrupt for External lines 4 to 15
  * @param  None
  * @retval None
  */
void EXTI4_15_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}

static void Hard_fault_handler_c(unsigned int* hardfault_args)
{
   unsigned int stacked_r0;
   unsigned int stacked_r1;
   unsigned int stacked_r2;
   unsigned int stacked_r3;
   unsigned int stacked_r12;
   unsigned int stacked_lr;
   unsigned int stacked_pc;
   unsigned int stacked_psr;

   stacked_r0 = ((unsigned long) hardfault_args[0]);
   stacked_r1 = ((unsigned long) hardfault_args[1]);
   stacked_r2 = ((unsigned long) hardfault_args[2]);
   stacked_r3 = ((unsigned long) hardfault_args[3]);

   stacked_r12 = ((unsigned long) hardfault_args[4]);
   stacked_lr = ((unsigned long) hardfault_args[5]);
   stacked_pc = ((unsigned long) hardfault_args[6]);
   stacked_psr = ((unsigned long) hardfault_args[7]);

   DEBUG_PRINTF ("[Hard fault handler]\r\n");
   DEBUG_PRINTF ("R0 = 0x%x\r\n", stacked_r0);
   DEBUG_PRINTF ("R1 = 0x%x\r\n", stacked_r1);
   DEBUG_PRINTF ("R2 = 0x%x\r\n", stacked_r2);
   DEBUG_PRINTF ("R3 = 0x%x\r\n", stacked_r3);
   DEBUG_PRINTF ("R12 = 0x%x\r\n", stacked_r12);
   DEBUG_PRINTF ("LR = 0x%x\r\n", stacked_lr);
   DEBUG_PRINTF ("PC = 0x%x\r\n", stacked_pc);
   DEBUG_PRINTF ("PSR = 0x%x\r\n", stacked_psr);

   while(1);
}
/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
