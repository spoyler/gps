/**
  ******************************************************************************
  * @file    stm32l0xx_nucleo_ihm04a1.c
  * @author  IPC Rennes
  * @version V1.0.0
  * @date    January 06, 2015
  * @brief   BSP driver for x-nucleo-ihm04a1 Nucleo extension board 
  *  (based on L6206)
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
#include "stm32l0xx_nucleo_ihm04a1.h"

/** @addtogroup BSP
  * @{
  */ 

/** @defgroup STM32L0XX_NUCLEO_IHM04A1
  * @{
  */   
    
/* Private constants ---------------------------------------------------------*/    

/** @defgroup IHM04A1_Private_Constants
  * @{
  */   
    
/// Timer Prescaler
#define TIMER_PRESCALER (1)


/// MCU wait time in ms after power bridges are enabled
#define BSP_MOTOR_CONTROL_BOARD_BRIDGE_TURN_ON_DELAY    (20)


/**
  * @}
  */ 

/* Private variables ---------------------------------------------------------*/

/** @defgroup IHM04A1_Board_Private_Variables
  * @{
  */       

/// Timer handler for PWM of Input 1 Bridge A 
TIM_HandleTypeDef hTimPwm1A;
/// imer handler for PWM of Input 2 Bridge A 
TIM_HandleTypeDef hTimPwm2A;
/// Timer handler for PWM of Input 1 Bridge B 
TIM_HandleTypeDef hTimPwm1B;
/// Timer handler for PWM of Input 2 Bridge B 
TIM_HandleTypeDef hTimPwm2B;
/**
  * @}
  */ 

/** @defgroup IHM04A1_Board_Private_Function_Prototypes
  * @{
  */   
   
/**
  * @}
  */


/** @defgroup  IHM04A1_Board_Private_Functions
  * @{
  */   

/******************************************************//**
 * @brief This function provides an accurate delay in milliseconds
 * @param[in] delay  time length in milliseconds
  * @retval None
 **********************************************************/
void BSP_MotorControlBoard_Delay(uint32_t delay)
{
  HAL_Delay(delay);
}

/******************************************************//**
 * @brief Disable the power bridges (leave the output bridges HiZ)
 * @param[in]  bridgeId (from 0 for bridge A to 1 for bridge B)
 * @retval None
 **********************************************************/
void BSP_MotorControlBoard_DisableBridge(uint8_t bridgeId)
{
 
    
}

/******************************************************//**
 * @brief Enable the power bridges (leave the output bridges HiZ)
 * @param[in]  bridgeId (from 0 for bridge A to 1 for bridge B)
 * @param[in]  addDelay if different from 0, a delay is added after bridge activation 
 * @retval None
 **********************************************************/
void BSP_MotorControlBoard_EnableBridge(uint8_t bridgeId, uint8_t addDelay)
{
  
}

/******************************************************//**
 * @brief  Returns the FLAG pin state.
 * @param[in]  bridgeId (from 0 for bridge A to 1 for bridge B)
 * @retval The FLAG pin value.
 **********************************************************/
uint32_t BSP_MotorControlBoard_GetFlagPinState(uint8_t bridgeId)
{
  
}

/******************************************************//**
 * @brief  Initiliases the GPIOs used by the L6206s
 * @param[in] None
 * @retval None
  **********************************************************/
void BSP_MotorControlBoard_GpioInit(void)
{
  
}

/******************************************************//**
 * @brief  Reset the PWM for the specified brigde input
 * @param[in] brigeInput 0 for input 1A, 1 for input 2A,
 * 2 for input 1B, 3 for input 2B
 * @retval None
 **********************************************************/
void BSP_MotorControlBoard_PwmDeInit(uint8_t brigeInput)
{
 
}

/******************************************************//**
 * @brief  Set the PWM frequency the for the specified bridge input
 * @param[in] brigeInput 0 for input 1A, 1 for input 2A,
 * 2 for input 1B, 3 for input 2B
 * @retval None
 **********************************************************/
void BSP_MotorControlBoard_PwmInit(uint8_t brigeInput)
{
  TIM_OC_InitTypeDef sConfigOC;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_HandleTypeDef *pHTim;
  uint32_t  channel;

  switch (brigeInput)
  {
  case 0:
  default:
      pHTim = &hTimPwm1A;
      pHTim->Instance = BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1A;
      pHTim->Channel = BSP_MOTOR_CONTROL_BOARD_HAL_ACT_CHAN_TIMER_PWM1A;
      channel = BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM1A;
      break;
    case  1:
      pHTim = &hTimPwm2A;
      pHTim->Instance = BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2A;
      pHTim->Channel = BSP_MOTOR_CONTROL_BOARD_HAL_ACT_CHAN_TIMER_PWM2A;
      channel = BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM2A;
      break;
    case 2:
      pHTim = &hTimPwm1B;
      pHTim->Instance = BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1B;
      pHTim->Channel = BSP_MOTOR_CONTROL_BOARD_HAL_ACT_CHAN_TIMER_PWM1B;
      channel = BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM1B;
      break;
    case 3:
      pHTim = &hTimPwm2B;
      pHTim->Instance = BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2B;
      pHTim->Channel = BSP_MOTOR_CONTROL_BOARD_HAL_ACT_CHAN_TIMER_PWM2B;
      channel = BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM2B;
      break;
  }
  pHTim->Init.Prescaler = TIMER_PRESCALER -1;
  pHTim->Init.CounterMode = TIM_COUNTERMODE_UP;
  pHTim->Init.Period = 0;
  pHTim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_PWM_Init(pHTim);
  
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(pHTim, &sConfigOC, channel);
  
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(pHTim, &sMasterConfig);
}

/******************************************************//**
 * @brief  Sets the frequency of PWM used for bridges inputs
 * @param[in] brigeInput 0 for input 1A, 1 for input 2A,
 * 2 for input 1B,  3 for input 2B
 * @param[in] newFreq in Hz
 * @retval None
 * @note The frequency is directly the current speed of the device
 **********************************************************/
void BSP_MotorControlBoard_PwmSetFreq(uint8_t brigeInput, uint32_t newFreq,uint8_t duty)
{
  uint32_t sysFreq = HAL_RCC_GetSysClockFreq();
  TIM_HandleTypeDef *pHTim;
  uint32_t period;
  uint32_t pulse;
  uint32_t channel;
  
  switch (brigeInput)
  {
    case 0:
    default:
      pHTim = &hTimPwm1A;
      pHTim->Instance = BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1A;
      channel = BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM1A;
      break;
    case  1:
      pHTim = &hTimPwm2A;
      pHTim->Instance = BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2A;
      channel = BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM2A;
      break;
    case 2:
      pHTim = &hTimPwm1B;
      pHTim->Instance = BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1B;
      channel = BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM1B;
      break;
    case 3:
      pHTim = &hTimPwm2B;
      pHTim->Instance = BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2B;
      channel = BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM2B;
      break;
}

   period = (sysFreq/ (TIMER_PRESCALER * newFreq)) - 1;
  
  __HAL_TIM_SetAutoreload(pHTim, period);
  
  if (duty == 0) 
{ 
    pulse = 0 ;
}
  else 
{
    if (duty > 100) duty = 100;  
    pulse = period * duty /100 +1;
  }    
  __HAL_TIM_SetCompare(pHTim, channel, pulse);
  HAL_TIM_PWM_Start_IT(pHTim, channel);  
}

/******************************************************//**
 * @brief  Stops the PWM uses for the specified brige input
 * @param[in] brigeInput 0 for input 1A, 1 for input 2A,
 * 2 for input 1B, 3 for input 2B
 * @retval None
 **********************************************************/
void BSP_MotorControlBoard_PwmStop(uint8_t bridgeInput)
{
  switch (bridgeInput)
  {
    case 0:
       HAL_TIM_PWM_Stop(&hTimPwm1A,BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM1A);
      break;
    case 1:
      HAL_TIM_PWM_Stop(&hTimPwm2A,BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM2A);
      break;
    case  2:
      HAL_TIM_PWM_Stop(&hTimPwm1B,BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM1B);
      break;
    case  3:
      HAL_TIM_PWM_Stop(&hTimPwm2B,BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM2B);
      break;
    default:
      break;//ignore error
    }
}

/**
  * @}
  */

/**
  * @}
  */    

/**
  * @}
  */ 
    
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
