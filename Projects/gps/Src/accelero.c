// Header:		accelero.h
// File Name: accelero.c
// Author:		andrey sidorov
// Date:			07.03.2016


#include <stdio.h>
#include "accelero.h"
#include "debug.h"
#include "rtc.h"

static void *LSM6DS3_X_0_handle = NULL;
static void *GYRO_handle = NULL;

volatile uint8_t mems_int1_detected = 0;
volatile uint8_t mems_int2_detected = 0;
volatile uint8_t state = 0;
volatile uint8_t return_state = 0;
volatile uint8_t free_fall_detect = 0;

				 
SensorAxesRaw_t acc_data;
SensorAxesRaw_t gyro_data;

void ACCELERO_Init()
{
	// GYRO INit
	BSP_GYRO_Init( GYRO_SENSORS_AUTO, &GYRO_handle );
	BSP_GYRO_Sensor_Enable(GYRO_handle);
	
	// ACCELERO_Init
	BSP_ACCELERO_Init( LSM6DS3_X_0, &LSM6DS3_X_0_handle );
	BSP_ACCELERO_Sensor_Enable( LSM6DS3_X_0_handle );
	
	BSP_ACCELERO_Enable_Free_Fall_Detection_Ext( LSM6DS3_X_0_handle );
	
	
	LSM6DS3_ACC_GYRO_W_INACTIVITY_ON(&LSM6DS3_X_0_handle, LSM6DS3_ACC_GYRO_INACTIVITY_ON_DISABLED);
	HAL_Delay(500);
	LSM6DS3_ACC_GYRO_W_INACTIVITY_ON(&LSM6DS3_X_0_handle, LSM6DS3_ACC_GYRO_INACTIVITY_ON_ENABLED);
	LSM6DS3_ACC_GYRO_W_SLEEP_DUR(LSM6DS3_X_0_handle, 7);
	LSM6DS3_ACC_GYRO_W_WAKE_DUR(LSM6DS3_X_0_handle, 2);
	LSM6DS3_ACC_GYRO_W_WK_THS(LSM6DS3_X_0_handle, 2);
	
	LSM6DS3_ACC_GYRO_W_SleepEvOnInt1(LSM6DS3_X_0_handle, LSM6DS3_ACC_GYRO_INT1_SLEEP_ENABLED);
	
}


void Accelero_Task()
{
	uint8_t  status = 0;
	GYRO_Drv_t *driver_gyro = NULL;
	ACCELERO_Drv_t *driver_acc = NULL;
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
	if (mems_int1_detected)
	{
		mems_int1_detected = 0;
		{	
			RTC_CalendarShow();
			return_state = state;

			if (return_state)
			{
				DEBUG_PRINTF("Active state\r\n");
			}
			else 
			{				
				DEBUG_PRINTF("Sleep state\r\n");
			}
		}
	}
	
	
	

	driver_gyro = ( GYRO_Drv_t * )((DrvContextTypeDef *)(GYRO_handle))->pVTable;	
	driver_acc = ( ACCELERO_Drv_t * )((DrvContextTypeDef *)(LSM6DS3_X_0_handle))->pVTable;
	
	driver_gyro->Get_Axes_Status(LSM6DS3_X_0_handle, &status);	
	// data from accelerometr
	if (status)
	{
		driver_gyro->Get_AxesRaw(LSM6DS3_X_0_handle, &acc_data);
/*	DEBUG_PRINTF("gyro_data_x = %d\t\t", gyro_data.AXIS_X);
		DEBUG_PRINTF("gyro_data_y = %d\t\t", gyro_data.AXIS_Y);
		DEBUG_PRINTF("gyro_data_z = %d\r\n", gyro_data.AXIS_Z);
*/		
	}

	driver_acc->Get_Axes_Status(LSM6DS3_X_0_handle, &status);	
	// data from gyroscope
	if (status)
	{
		driver_acc->Get_AxesRaw(LSM6DS3_X_0_handle, &gyro_data);
/*	DEBUG_PRINTF("acc_data_x = %d\t\t", acc_data.AXIS_X);
		DEBUG_PRINTF("acc_data_y = %d\t\t", acc_data.AXIS_Y);
		DEBUG_PRINTF("acc_data_z = %d\r\n", acc_data.AXIS_Z);
*/		
	}
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

uint8_t Get_Free_Fall_State()
{
	uint8_t ret_value = free_fall_detect;
	
	if (free_fall_detect)
		free_fall_detect = 0;
	
	return ret_value;	
}

void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
	if ( GPIO_Pin == M_INT1_PIN )
  {
		mems_int1_detected = 1;
		state = !state;
  }
	else
		if (GPIO_Pin == M_INT2_PIN)
		{
			mems_int2_detected = 1;
		}
}
