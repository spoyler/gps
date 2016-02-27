/**
 ******************************************************************************
 * @file    x_nucleo_iks01a1.c
 * @author  MEMS Application Team
 * @version V2.0.0
 * @date    10-December-2015
 * @brief   This file provides X_NUCLEO_IKS01A1 MEMS shield board specific functions
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

#include "x_nucleo_iks01a1.h"



/** @addtogroup BSP BSP
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1 X_NUCLEO_IKS01A1
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1_IO IO
 * @{
 */

/** @addtogroup X_NUCLEO_IKS01A1_IO_Private_Variables Private variables
 * @{
 */

static uint32_t SPI_EXPBD_Timeout = 100;    /*<! Value of Timeout when SPI communication fails */
static SPI_HandleTypeDef SPI_EXPBD_Handle;
static SPI_HandleTypeDef SpiHandle;

#define RX_TX_BUFFER_SIZE 32
static uint8_t spi_rx_buffer[RX_TX_BUFFER_SIZE];
static uint8_t spi_tx_buffer[RX_TX_BUFFER_SIZE];

/**
 * @}
 */

/* Link function for sensor peripheral */
uint8_t Sensor_IO_Write( void *handle, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite );
uint8_t Sensor_IO_Read( void *handle, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead );

static void SPI_EXPBD_MspInit( void );
static void SPI_EXPBD_Error( uint8_t Addr );
static uint8_t SPI_EXPBD_ReadData( uint8_t Addr, uint8_t Reg, uint8_t* pBuffer, uint16_t Size );
static uint8_t SPI_EXPBD_WriteData( uint8_t Addr, uint8_t Reg, uint8_t* pBuffer, uint16_t Size );
static uint8_t SPI_EXPBD_Init( void );

/** @addtogroup X_NUCLEO_IKS01A1_IO_Public_Functions Public functions
 * @{
 */

/**
 * @brief  Configures sensor SPI interface.
 * @param  None
 * @retval COMPONENT_OK in case of success
 * @retval COMPONENT_ERROR in case of failure
 */
DrvStatusTypeDef Sensor_IO_Init( void )
{

  if ( SPI_EXPBD_Init() )
  {
    return COMPONENT_ERROR;
  }
  else
  {
    return COMPONENT_OK;
  }
}



/**
 * @brief  Configures sensor interrupts interface for LSM6DS0 sensor).
 * @param  None
 * @retval COMPONENT_OK in case of success
 * @retval COMPONENT_ERROR in case of failure
 */
DrvStatusTypeDef LSM6DS0_Sensor_IO_ITConfig( void )
{

  /*Not implemented yet*/
  
  return COMPONENT_OK;
}



/**
 * @brief  Configures sensor interrupts interface for LSM6DS3 sensor.
 * @param  None
 * @retval COMPONENT_OK in case of success
 * @retval COMPONENT_ERROR in case of failure
 */
DrvStatusTypeDef LSM6DS3_Sensor_IO_ITConfig( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__GPIOA_CLK_ENABLE();
	
	 /* Configure GPIO PINs to detect Interrupts */
  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* Enable and set EXTI Interrupt priority */
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0x00, 0x00);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
  
  return COMPONENT_OK;
}

/**
 * @}
 */


/** @addtogroup X_NUCLEO_IKS01A1_IO_Private_Functions Private functions
 * @{
 */



/******************************* Link functions *******************************/


/**
 * @brief  Writes a buffer to the sensor
 * @param  handle instance handle
 * @param  WriteAddr specifies the internal sensor address register to be written to
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToWrite number of bytes to be written
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
uint8_t Sensor_IO_Write( void *handle, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite )
{
  DrvContextTypeDef *ctx = (DrvContextTypeDef *)handle;
  
  /* call SPI_EXPBD Read data bus function */
  if ( SPI_EXPBD_WriteData( ctx->address, WriteAddr, pBuffer, nBytesToWrite ) )
  {
    return 1;
  }
  else
  {
    return 0;
  }
}



/**
 * @brief  Reads a from the sensor to buffer
 * @param  handle instance handle
 * @param  ReadAddr specifies the internal sensor address register to be read from
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToRead number of bytes to be read
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
uint8_t Sensor_IO_Read( void *handle, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead )
{
  DrvContextTypeDef *ctx = (DrvContextTypeDef *)handle;
  
	/* call SPI_EXPBD Read data bus function */
  if ( SPI_EXPBD_ReadData( ctx->address, ReadAddr, pBuffer, nBytesToRead ) )
  {
    return 1;
  }
  else
  {
    return 0;
  }
}



/******************************* SPI Routines *********************************/

/**
 * @brief  Configures SPI interface.
 * @param  None
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
static uint8_t SPI_EXPBD_Init( void )
{
	
	SPI_EXPBD_MspInit();
	
	SpiHandle.Instance               = SPI2;
  
  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiHandle.Init.CLKPhase          = SPI_PHASE_2EDGE;
  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
  SpiHandle.Init.CRCCalculation    = 0;//SPI_CRCCALCULATION_DISABLE; //TODO
  SpiHandle.Init.CRCPolynomial     = 0;
  SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.NSS               = SPI_NSS_SOFT;
  SpiHandle.Init.TIMode            = 0;//SPI_TIMODE_DISABLE; //TODO
  
  SpiHandle.Init.Mode = SPI_MODE_MASTER;

  HAL_SPI_Init(&SpiHandle);
	
	__HAL_SPI_ENABLE(&SpiHandle);
	
	return 0;
}



/**
 * @brief  Write data to the register of the device through BUS
 * @param  Addr Device address on BUS
 * @param  Reg The target register address to be written
 * @param  pBuffer The data to be written
 * @param  Size Number of bytes to be written
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
static uint8_t SPI_EXPBD_WriteData( uint8_t Addr, uint8_t Reg, uint8_t* pBuffer, uint16_t Size )
{

  HAL_StatusTypeDef status = HAL_OK;
	
	spi_tx_buffer[0] = Reg;
	
	for (int i = 0; (i < Size) && (i < RX_TX_BUFFER_SIZE); ++i)
	{
		spi_tx_buffer[i + 1] = pBuffer[i];
	}
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
	status = HAL_SPI_TransmitReceive(&SpiHandle, spi_tx_buffer, spi_rx_buffer, Size + 1, SPI_EXPBD_Timeout);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	
	return status;
                              
  /* Check the communication status */
  if( status != HAL_OK )
  {
  
    /* Execute user& timeout callback */
    SPI_EXPBD_Error( Addr );
    return 1;
  }
  else
  {
    return 0;
  }
}



/**
 * @brief  Read a register of the device through BUS
 * @param  Addr Device address on BUS
 * @param  Reg The target register address to read
 * @param  pBuffer The data to be read
 * @param  Size Number of bytes to be read
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
static uint8_t SPI_EXPBD_ReadData( uint8_t Addr, uint8_t Reg, uint8_t* pBuffer, uint16_t Size )
{

  HAL_StatusTypeDef status = HAL_OK;
	
	spi_tx_buffer[0] = Reg | (1 << 7);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
	//status = HAL_SPI_TransmitReceive(&SpiHandle, spi_tx_buffer, pBuffer, Size + 1, SPI_EXPBD_Timeout);
	
	while (		(SpiHandle.Instance->SR & SPI_FLAG_OVR)
					||(SpiHandle.Instance->SR & SPI_FLAG_RXNE))
	{
		pBuffer[0] = SpiHandle.Instance->DR;
	}
		
	for (int i = 0; i < Size + 1 && (i < RX_TX_BUFFER_SIZE); ++i)
	{
		while(!(SpiHandle.Instance->SR & SPI_FLAG_TXE));
		SpiHandle.Instance->DR = spi_tx_buffer[i];
		if (SpiHandle.Instance->SR & SPI_FLAG_RXNE)
			pBuffer[i] = SpiHandle.Instance->DR;
	}
	
	while((SpiHandle.Instance->SR & SPI_FLAG_BSY))
	{
		if (		(SpiHandle.Instance->SR & SPI_FLAG_OVR)
					||(SpiHandle.Instance->SR & SPI_FLAG_RXNE))
		{
			pBuffer[0] = SpiHandle.Instance->DR;
		}
	}
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	
	if ((SpiHandle.Instance->SR & SPI_FLAG_OVR)
			||(SpiHandle.Instance->SR & SPI_FLAG_RXNE))
	{
			pBuffer[0] = SpiHandle.Instance->DR;
	}
	
	return status;
                             
  /* Check the communication status */
  if( status != HAL_OK )
  {
  
    /* Execute user timeout callback */
    SPI_EXPBD_Error( Addr );
    return 1;
  }
  else
  {
    return 0;
  }
}



/**
 * @brief  Manages error callback by re-initializing SPI
 * @param  Addr SPI Address
 * @retval None
 */
static void SPI_EXPBD_Error( uint8_t Addr )
{

  /* De-initialize the SPI comunication bus */
  HAL_SPI_DeInit( &SPI_EXPBD_Handle );
  
  /* Re-Initiaize the SPI comunication bus */
  SPI_EXPBD_Init();
}



/**
 * @brief SPI MSP Initialization
 * @param None
 * @retval None
 */

static void SPI_EXPBD_MspInit( void )
{
	
		GPIO_InitTypeDef GPIO_InitStruct;
	
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	
	
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
	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
  
  /* Enable the SPI2 peripheral clock */
  __SPI2_CLK_ENABLE();
  
  /* Force the SPI peripheral clock reset */
  NUCLEO_SPI_FORCE_RESET();
  
  /* Release the SPI peripheral clock reset */
  NUCLEO_SPI_RELEASE_RESET();
  
  /* Enable and set SPI_EXPBD Interrupt to the highest priority */
  HAL_NVIC_SetPriority(SPI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(SPI2_IRQn);
  
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

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
