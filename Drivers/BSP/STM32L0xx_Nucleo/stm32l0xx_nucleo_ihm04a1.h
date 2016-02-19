/** 
  ******************************************************************************
  * @file    stm32l0xx_nucleo_ihm04a1.h
  * @author  IPC Rennes
  * @version V1.0.0
  * @date    January 06, 2015
  * @brief   Header for BSP driver for x-nucleo-ihm04a1 Nucleo extension board 
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32L0XX_NUCLEO_IHM04A1_H
#define __STM32L0XX_NUCLEO_IHM04A1_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_nucleo.h"
   
/** @addtogroup BSP
  * @{
  */   
   
/** @addtogroup STM32L0XX_NUCLEO_IHM04A1
  * @{   
  */   
   
/* Exported Constants --------------------------------------------------------*/
   
/** @defgroup IHM04A1_Exported_Constants
  * @{
  */   
   
/******************************************************************************/
/* USE_STM32L0XX_NUCLEO                                                       */
/******************************************************************************/

 /** @defgroup Constants_For_STM32L0XX_NUCLEO  
* @{
*/   
/// Interrupt line used for L6206 Over Current Detection and over Temperature On Bridge A
#define EXTI_FLAG_A_IRQn           (EXTI4_15_IRQn)

/// Interrupt line used for L6206 Over Current Detection and over Temperature On Bridge B
#define EXTI_FLAG_B_IRQn           (EXTI0_1_IRQn)   
   
/// Timer used for PWM_1A
#define BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1A      (TIM22)

/// Timer used for PWM_2A
#define BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2A      (TIM22)

/// Timer used for PWM_1B
#define BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1B      (TIM2)

/// Timer used for PWM_2B
#define BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2B      (TIM2)   

/// Channel Timer used for PWM_1A
#define BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM1A      (TIM_CHANNEL_1)

/// Channel Timer used for PWM_2A
#define BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM2A      (TIM_CHANNEL_2)

/// Channel Timer used for PWM_1B
#define BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM1B      (TIM_CHANNEL_1)

/// Channel Timer used for PWM_2B
#define BSP_MOTOR_CONTROL_BOARD_CHAN_TIMER_PWM2B      (TIM_CHANNEL_2)

/// HAL Active Channel Timer used for PWM_1A
#define BSP_MOTOR_CONTROL_BOARD_HAL_ACT_CHAN_TIMER_PWM1A      (HAL_TIM_ACTIVE_CHANNEL_1)

/// HAL Active Channel Timer used for PWM_2A
#define BSP_MOTOR_CONTROL_BOARD_HAL_ACT_CHAN_TIMER_PWM2A      (HAL_TIM_ACTIVE_CHANNEL_2)

/// HAL Active Channel Timer used for PWM_1B
#define BSP_MOTOR_CONTROL_BOARD_HAL_ACT_CHAN_TIMER_PWM1B      (HAL_TIM_ACTIVE_CHANNEL_1)

/// HAL Active Channel Timer used for PWM_2B
#define BSP_MOTOR_CONTROL_BOARD_HAL_ACT_CHAN_TIMER_PWM2B      (HAL_TIM_ACTIVE_CHANNEL_2)
   
/// Timer Clock Enable for PWM_1A
#define __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1A_CLCK_ENABLE()  __TIM22_CLK_ENABLE()

/// Timer Clock Enable for PWM_2A
#define __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2A_CLCK_ENABLE()  __TIM22_CLK_ENABLE()

/// Timer Clock Enable for PWM_1B
#define __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1B_CLCK_ENABLE()  __TIM2_CLK_ENABLE()

/// Timer Clock Enable for PWM_2B
#define __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2B_CLCK_ENABLE()  __TIM2_CLK_ENABLE()
   
/// Timer Clock Enable for PWM_1A
#define __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1A_CLCK_DISABLE()  __TIM22_CLK_DISABLE()

/// Timer Clock Enable for PWM_2A
#define __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2A_CLCK_DISABLE()  __TIM22_CLK_DISABLE()

/// Timer Clock Enable for PWM_1B
#define __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1B_CLCK_DISABLE()  __TIM2_CLK_DISABLE()

/// Timer Clock Enable for PWM_2B
#define __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2B_CLCK_DISABLE()  __TIM2_CLK_DISABLE()

/// PWM1A GPIO alternate function 
#define BSP_MOTOR_CONTROL_BOARD_AFx_TIMx_PWM1A  (GPIO_AF4_TIM22)

/// PWM2A GPIO alternate function 
#define BSP_MOTOR_CONTROL_BOARD_AFx_TIMx_PWM2A  (GPIO_AF4_TIM22)

/// PWM1A GPIO alternate function 
#define BSP_MOTOR_CONTROL_BOARD_AFx_TIMx_PWM1B  (GPIO_AF2_TIM2)

/// PWM2A GPIO alternate function 
#define BSP_MOTOR_CONTROL_BOARD_AFx_TIMx_PWM2B  (GPIO_AF2_TIM2)
   
 /**
* @}
*/

#define DEBUG_PRINTF(...) printf(__VA_ARGS__)


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32L0XX_NUCLEO_IHM04A1_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
