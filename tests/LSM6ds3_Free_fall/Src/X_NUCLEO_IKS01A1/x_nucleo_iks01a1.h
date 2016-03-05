/**
  ******************************************************************************
  * @file    x_nucleo_iks01a1.h
  * @author  MEMS Application Team
  * @version V2.0.0
  * @date    10-December-2015
  * @brief   This file contains definitions for the x_nucleo_iks01a1.c
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
#ifndef __X_NUCLEO_IKS01A1_H
#define __X_NUCLEO_IKS01A1_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "stm32l0xx_hal.h"
#include "accelerometer.h"
#include "gyroscope.h"

/* Timing samples for L0 with SYSCLK 32MHz set in SystemClock_Config() */
#if (defined (USE_STM32L0XX_NUCLEO))
#define NUCLEO_I2C_EXPBD_TIMING_100KHZ       0x10A13E56 /* Analog Filter ON, Rise Time 400ns, Fall Time 100ns */
#define NUCLEO_I2C_EXPBD_TIMING_400KHZ       0x00B1112E /* Analog Filter ON, Rise Time 250ns, Fall Time 100ns */
#endif /* USE_STM32L0XX_NUCLEO */

/* Timing samples for L4 with SYSCLK 80MHz set in SystemClock_Config() */
#if (defined (USE_STM32L4XX_NUCLEO))
#define NUCLEO_I2C_EXPBD_TIMING_1000KHZ      0x00D00E28 /* Analog Filter ON, Rise time 120ns, Fall time 25ns */
#endif /* USE_STM32L4XX_NUCLEO */

/* I2C peripheral configuration defines */

#define NUCLEO_I2C_EXPBD_CLK_ENABLE()               __I2C1_CLK_ENABLE()
#define NUCLEO_I2C_EXPBD_SCL_SDA_GPIO_CLK_ENABLE()  __GPIOB_CLK_ENABLE()
#define NUCLEO_I2C_EXPBD_SCL_SDA_AF                 GPIO_AF4_I2C1
#define NUCLEO_I2C_EXPBD_SCL_SDA_GPIO_PORT          GPIOB
#define NUCLEO_I2C_EXPBD_SCL_PIN                    GPIO_PIN_8
#define NUCLEO_I2C_EXPBD_SDA_PIN                    GPIO_PIN_9

#define NUCLEO_SPI_FORCE_RESET()              __SPI2_FORCE_RESET()
#define NUCLEO_SPI_RELEASE_RESET()            __SPI2_RELEASE_RESET()




// ready for use
#define M_INT1_GPIO_PORT           GPIOA
#define M_INT1_GPIO_CLK_ENABLE()   __GPIOA_CLK_ENABLE()
#define M_INT1_GPIO_CLK_DISABLE()  __GPIOA_CLK_DISABLE()
#define M_INT1_PIN                 GPIO_PIN_10

#if (defined (USE_STM32L0XX_NUCLEO))
#define M_INT1_EXTI_IRQn           EXTI4_15_IRQn
#endif

// ready for use
#define M_INT2_GPIO_PORT           GPIOA
#define M_INT2_GPIO_CLK_ENABLE()   __GPIOB_CLK_ENABLE()
#define M_INT2_GPIO_CLK_DISABLE()  __GPIOB_CLK_DISABLE()
#define M_INT2_PIN                 GPIO_PIN_9

#if (defined (USE_STM32L0XX_NUCLEO))
#define M_INT2_EXTI_IRQn           EXTI4_15_IRQn
#endif

/**
  * @}
  */

/** @addtogroup X_NUCLEO_IKS01A1_IO_Public_FunctionPrototypes Public function prototypes
 * @{
 */
DrvStatusTypeDef Sensor_IO_Init( void );
DrvStatusTypeDef LSM6DS0_Sensor_IO_ITConfig( void );
DrvStatusTypeDef LSM6DS3_Sensor_IO_ITConfig( void );
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

#endif /* __X_NUCLEO_IKS01A1_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
