// Header:		adc.h
// File Name: adc.h
// Author:		andrey sidorov
// Date:			07.03.2016


#include "adc.h"
#include "debug.h"

/* ADC handle declaration */
ADC_HandleTypeDef             AdcHandle;

/* ADC channel configuration structure declaration */
ADC_ChannelConfTypeDef        sConfig;

/* Variable used to get converted value */
__IO uint32_t uwADCxConvertedValue = 0;

const uint8_t num_ch = 4;
const uint8_t kDeleyBeforeADC = 1; 		// in milisecons
uint16_t adc_result[num_ch];

int32_t dif_adc_result[2];

void ADC_Init()
{
	  /* ADC1 Periph clock enable */
  __ADC1_CLK_ENABLE();
	
	  /* ### - 1 - Initialize ADC peripheral #################################### */
  /*
   *  Instance                  = ADC1.
   *  OversamplingMode          = Disabled
   *  ClockPrescaler            = PCLK clock with no division.
   *  LowPowerAutoPowerOff      = Disabled (For this exemple continuous mode is enabled with software start)
   *  LowPowerFrequencyMode     = Enabled (To be enabled only if ADC clock is lower than 2.8MHz) 
   *  LowPowerAutoWait          = Disabled (New conversion starts only when the previous conversion is completed)       
   *  Resolution                = 12 bit (increased to 16 bit with oversampler)
   *  SamplingTime              = 7.5 cycles od ADC clock.
   *  ScanConvMode              = Forward
   *  DataAlign                 = Right
   *  ContinuousConvMode        = Enabled
   *  DiscontinuousConvMode     = Disabled
   *  ExternalTrigConvEdge      = None (Software start)
   *  EOCSelection              = End Of Conversion event
   *  DMAContinuousRequests     = DISABLE
   */

  AdcHandle.Instance = ADC1;
  
  AdcHandle.Init.OversamplingMode      = DISABLE;
  
  AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV32;
  AdcHandle.Init.LowPowerAutoOff  		 = DISABLE;
  AdcHandle.Init.LowPowerFrequencyMode = ENABLE;
  AdcHandle.Init.LowPowerAutoWait      = DISABLE;
    
  AdcHandle.Init.Resolution            = ADC_RESOLUTION12b;
  AdcHandle.Init.SamplingTime          = ADC_SAMPLETIME_7CYCLES_5;
  AdcHandle.Init.ScanDirection         = ADC_SCAN_DIRECTION_UPWARD;
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  AdcHandle.Init.ContinuousConvMode    = ENABLE;
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIG_EDGE_NONE;
  AdcHandle.Init.EOCSelection          = EOC_SINGLE_CONV;
  AdcHandle.Init.DMAContinuousRequests = DISABLE;
 
  /* Initialize ADC peripheral according to the passed parameters */
	HAL_ADC_Init(&AdcHandle);
  
  
  /* ### - 2 - Start calibration ############################################ */
  HAL_ADCEx_Calibration_Start(&AdcHandle, ADC_SINGLE_ENDED);
	
	
	sConfig.Channel = ADC_CHANNEL_4;    
  HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);
	sConfig.Channel = ADC_CHANNEL_5; 
  HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);
	sConfig.Channel = ADC_CHANNEL_6;    
  HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);
	sConfig.Channel = ADC_CHANNEL_7;    
  HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);
}

void ADC_Task()
{
	int i = 0;
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_Delay(kDeleyBeforeADC);
	
	
  /*##- 4- Start the conversion process #######################################*/  
  HAL_ADC_Start(&AdcHandle);
	
	while(1)
	{
  	
    /* Check if the continous conversion of regular channel is finished */
    if ((AdcHandle.Instance->ISR & HAL_ADC_STATE_EOC) == HAL_ADC_STATE_EOC)
    {
      /*##-6- Get the converted value of regular channel  ########################*/
      adc_result[i++] = HAL_ADC_GetValue(&AdcHandle);
						
			
			if (i == num_ch)
			{
				HAL_ADC_Stop(&AdcHandle);
				GPIO_InitStruct.Pin = GPIO_PIN_1;
				GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
				HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);				
				break;
			}
    }
	}
}

void ADC_Debug()
{
	int i = 0;
	ADC_Task();
	while(i < num_ch)
	{
		DEBUG_PRINTF("adc[%d] = 0x%x\r\n", i, adc_result[i]);
		i++;
	}	
	
	DEBUG_PRINTF("Charging state: ");
	if (GetChargingState())
		DEBUG_PRINTF("ON\r\n");
	else
		DEBUG_PRINTF("OFF\r\n");
}

int32_t * Get_ADC_Data()
{
	dif_adc_result[0] = adc_result[0] - adc_result[1];
	dif_adc_result[1] = adc_result[2] - adc_result[3];
	
	return 	dif_adc_result;
}


