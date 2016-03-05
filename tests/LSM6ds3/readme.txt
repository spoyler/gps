/** 
  @page Motion_Control_IHM04A1_ExampleFor4UniDirMotors  
  Example of 4 unidirectionnal Brush DC motors driving with an L6206 
  (used via expansion board IHM04A1)
  
  @verbatim
  ******************** (C) COPYRIGHT 2015 STMicroelectronics *******************
  * @file    Multi/Examples/MotionControl/IHM04A1_ExampleFor4UniDirMotors/readme.txt  
  * @author  IPC Rennes
  * @version V1.0.0
  * @date    January 06, 2015
  * @brief   Description of the example of 4 unidirectional Brush DC motor 
  * driving with L6206
  ******************************************************************************
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
  @endverbatim

@par Example Description 

This example describes how to use the L6206 FW library to drive 4 unidirectionnal 
brush DC motors. The demo sequence starts when the user button is pressed.
Each time, the user button is pressed, the demo step is changed.


@par Directory contents 

  - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Inc/stm32f4xx_hal_conf.h    HAL configuration file
  - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Inc/stm32l0xx_hal_conf.h    HAL configuration file
  - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Inc/stm32f4xx_it.h          Interrupt handlers header file
  - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Inc/stm32l0xx_it.h          Interrupt handlers header file
  - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Inc/main.h                  Header for main.c module
  - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Src/stm32f4xx_it.c          Interrupt handlers
  - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Src/stm32l0xx_it.c          Interrupt handlers    
  - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Src/main.c                  Main program
  - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Src/system_stm32f4xx.c      System source file
  - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Src/system_stm32l0xx.c      System source file
  - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Src/stm32f4xx_hal_msp.c     HAL MSP module  
  - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Src/stm32l0xx_hal_msp.c     HAL MSP module
  - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Src/clock_f4.c              Clock configuration
    - MotionControl/IHM04A1_ExampleFor4UniDirMotors/Src/clock_l0.c              Clock configuration


@par Hardware and Software environment

  This example requires :
    - Either a NUCLEO-F401RE, NUCLEO-L053R8 board : a STM32 Nucleo development board for STM32 either F4 or L0 series
    - an X-NUCLEO-IHM04A1 board: a dual full bridge driver  expansion board based on the L6206
    - 4 unidirectionnal BRUSH DC motors connected to the X-NUCLEO-IHM04A1 board.    
    Each motor has one lead connected to one of the bridge output, the other lead to the ground.
    The input bridges are not parallelised.

@par How to use it ? 

In order to make the program work, you must do the following :
 - Open your preferred toolchain 
 - Rebuild all files and load your image into target memory
 - Run the example

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
