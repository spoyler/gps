// Header:		accelero.h
// File Name: accelero.c
// Author:		andrey sidorov
// Date:			07.03.2016


#include <stdio.h>
#include "accelero.h"
#include "debug.h"
#include "rtc.h"
#include "command.h"

static void *LSM6DS3_X_0_handle = NULL;
static void *GYRO_handle = NULL;

volatile uint8_t mems_int1_detected = 0;
volatile uint8_t mems_int2_detected = 0;
volatile uint8_t acc_state = 0;	// after start, state of the accelerometer must be inactive
volatile uint8_t return_state = 0;
volatile uint8_t free_fall_detect = 0;
DrvStatusTypeDef accelero_init = COMPONENT_OK;

				 
SensorAxesRaw_t acc_data;
SensorAxesRaw_t gyro_data;

void ACCELERO_Init()
{
	DEBUG_PRINTF("Accelero Init...");
	// GYRO INit
	BSP_GYRO_DeInit(&GYRO_handle);
	accelero_init += BSP_GYRO_Init( GYRO_SENSORS_AUTO, &GYRO_handle );
	accelero_init += BSP_GYRO_Sensor_Enable(GYRO_handle);
	
	// ACCELERO_Init
	BSP_ACCELERO_DeInit(&LSM6DS3_X_0_handle );
	accelero_init += BSP_ACCELERO_Init( LSM6DS3_X_0, &LSM6DS3_X_0_handle );
	accelero_init += BSP_ACCELERO_Sensor_Enable( LSM6DS3_X_0_handle );
	
	accelero_init += BSP_ACCELERO_Enable_Free_Fall_Detection_Ext( LSM6DS3_X_0_handle );
	
	
	//LSM6DS3_ACC_GYRO_W_INACTIVITY_ON(&LSM6DS3_X_0_handle, LSM6DS3_ACC_GYRO_INACTIVITY_ON_DISABLED);
	//HAL_Delay(500);
	//LSM6DS3_ACC_GYRO_W_INACTIVITY_ON(&LSM6DS3_X_0_handle, LSM6DS3_ACC_GYRO_INACTIVITY_ON_ENABLED);
	//LSM6DS3_ACC_GYRO_W_SLEEP_DUR(LSM6DS3_X_0_handle, 7);
	//LSM6DS3_ACC_GYRO_W_WAKE_DUR(LSM6DS3_X_0_handle, 2);
	//LSM6DS3_ACC_GYRO_W_WK_THS(LSM6DS3_X_0_handle, 2);
	
	//LSM6DS3_ACC_GYRO_W_SleepEvOnInt1(LSM6DS3_X_0_handle, LSM6DS3_ACC_GYRO_INT1_SLEEP_ENABLED);
	
	//acc_state = LoadAcceleroStateFromBKP();
	accelero_init += BSP_ACCELERO_Enable_Wake_Up_Detection_Ext( LSM6DS3_X_0_handle );
	if (!accelero_init)
		DEBUG_PRINTF("Ok\r\n");
	else
		DEBUG_PRINTF("False\r\n");
}


void Accelero_Task()
{
	uint8_t  status = 0;
	GYRO_Drv_t *driver_gyro = NULL;
	ACCELERO_Drv_t *driver_acc = NULL;
	
	if (accelero_init == COMPONENT_OK)
	{
		// check interrupt from gyroscope
		if ( mems_int2_detected != 0 )
		{
			if ( BSP_ACCELERO_Get_Free_Fall_Detection_Status_Ext( LSM6DS3_X_0_handle, &status ) == COMPONENT_OK )
			{
				if ( status != 0 )
				{
					free_fall_detect = 1;
					DEBUG_PRINTF("Free Fall Detection!!!!\r\n");
				}
			}
			mems_int2_detected = 0;
		}
		//status = 0;
		//BSP_ACCELERO_Get_Wake_Up_Detection_Status_Ext( LSM6DS3_X_0_handle, &status );
		//if (status)
		
		if (mems_int1_detected)
		{
			mems_int1_detected = 0;
			{	
				DEBUG_PRINTF("Active state.\r\n");
				RTC_AlarmConfig(GetParamValue(WAIT_BEFOR_SLEEP), RTC_ALARM_A);
			}
		}
		
		driver_gyro = ( GYRO_Drv_t * )((DrvContextTypeDef *)(GYRO_handle))->pVTable;	
		driver_acc = ( ACCELERO_Drv_t * )((DrvContextTypeDef *)(LSM6DS3_X_0_handle))->pVTable;
		
		driver_gyro->Get_Axes_Status(LSM6DS3_X_0_handle, &status);	
		// data from accelerometr
		if (status)
		{
			driver_gyro->Get_AxesRaw(LSM6DS3_X_0_handle, &acc_data);
			if (Get_Debug_State(ACC))
			{
				DEBUG_PRINTF("gyro_data_x = %d ", gyro_data.AXIS_X);
				DEBUG_PRINTF("gyro_data_y = %d ", gyro_data.AXIS_Y);
				DEBUG_PRINTF("gyro_data_z = %d\r\n", gyro_data.AXIS_Z);
			}		
		}

		driver_acc->Get_Axes_Status(LSM6DS3_X_0_handle, &status);	
		// data from gyroscope
		if (status)
		{
			driver_acc->Get_AxesRaw(LSM6DS3_X_0_handle, &gyro_data);
			if (Get_Debug_State(ACC))
			{
				DEBUG_PRINTF("acc_data_x = %d ", acc_data.AXIS_X);
				DEBUG_PRINTF("acc_data_y = %d ", acc_data.AXIS_Y);
				DEBUG_PRINTF("acc_data_z = %d\r\n", acc_data.AXIS_Z);
			}
		}
	}
}
void ACC_Debug()
{
	Accelero_Task();
}


SensorAxesRaw_t * Get_ACC_Data()
{
	return &acc_data;
}

SensorAxesRaw_t * Get_GYRO_Data()
{
	return &gyro_data;
}

uint8_t Get_Accelero_State()
{
	return return_state;
}

void Set_Gyro_Sleep_Mode(uint8_t new_mode)
{
	if (new_mode)
		LSM6DS3_ACC_GYRO_W_SleepMode_G(LSM6DS3_X_0_handle, LSM6DS3_ACC_GYRO_SLEEP_G_ENABLED);
	else
		LSM6DS3_ACC_GYRO_W_SleepMode_G(LSM6DS3_X_0_handle, LSM6DS3_ACC_GYRO_SLEEP_G_DISABLED);
}

uint8_t Get_Free_Fall_State()
{
	uint8_t ret_value = free_fall_detect;
	
	if (free_fall_detect)
		free_fall_detect = 0;
	
	return ret_value;	
}

void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
	
	if (__HAL_PWR_GET_FLAG(PWR_FLAG_WU))
	{
		/* Clear Wake Up Flag */
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
		SetWakeupSource(WAKEUP_SOURCE_ACC);
	}
	
	if (GPIO_Pin == GPIO_PIN_2)
	{
		
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_RESET)
		{
			__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
			SetWakeupSource(WAKEUP_SOURCE_CHARGING);
			ChargingOn();
		}
		else
		{
			ChargingOFF();
		}
				
	}
		
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


void EnableWakeupDetection(void)
{
	BSP_ACCELERO_Enable_Wake_Up_Detection_Ext( LSM6DS3_X_0_handle );
}

void DisableWakeupDetection(void)
{
	BSP_ACCELERO_Disable_Wake_Up_Detection_Ext( LSM6DS3_X_0_handle );
}
