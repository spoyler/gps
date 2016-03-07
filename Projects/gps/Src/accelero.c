// Header:		accelero.h
// File Name: accelero.c
// Author:		andrey sidorov
// Date:			07.03.2016


#include "accelero.h"
#include "debug.h"

static void *LSM6DS3_X_0_handle = NULL;
volatile uint8_t mems_int1_detected = 0;
volatile uint8_t mems_int2_detected = 0;
volatile uint8_t state = 1;
				 uint8_t last_state = 1;
				 
int16_t acc_data[3] = {0};
int16_t gyro_data[3] = {0};

void ACCELERO_Init()
{
	BSP_ACCELERO_Init( LSM6DS3_X_0, &LSM6DS3_X_0_handle );
	BSP_ACCELERO_Sensor_Enable( LSM6DS3_X_0_handle );
	
	BSP_ACCELERO_Enable_Free_Fall_Detection_Ext( LSM6DS3_X_0_handle );
	
	LSM6DS3_ACC_GYRO_W_INACTIVITY_ON(&LSM6DS3_X_0_handle, LSM6DS3_ACC_GYRO_INACTIVITY_ON_DISABLED);
	
	LSM6DS3_ACC_GYRO_W_INACTIVITY_ON(&LSM6DS3_X_0_handle, LSM6DS3_ACC_GYRO_INACTIVITY_ON_ENABLED);
	LSM6DS3_ACC_GYRO_W_SLEEP_DUR(LSM6DS3_X_0_handle, 7);
	LSM6DS3_ACC_GYRO_W_WAKE_DUR(LSM6DS3_X_0_handle, 2);
	LSM6DS3_ACC_GYRO_W_WK_THS(LSM6DS3_X_0_handle, 2);
	
	LSM6DS3_ACC_GYRO_W_SleepEvOnInt1(LSM6DS3_X_0_handle, LSM6DS3_ACC_GYRO_INT1_SLEEP_ENABLED);
}


void Accelero_Task()
{
	uint8_t  status = 0;
	// check interrupt from gyroscope
	if ( mems_int2_detected != 0 )
	{
		if ( BSP_ACCELERO_Get_Free_Fall_Detection_Status_Ext( LSM6DS3_X_0_handle, &status ) == COMPONENT_OK )
		{
			if ( status != 0 )
			{
				DEBUG_PRINTF("Free Fall Detection!!!!\r\n");
			}
		}
		mems_int2_detected = 0;
	}
	if (last_state != state)
	{	
		last_state = state;
		if (state)
			DEBUG_PRINTF("Active state\r\n");
		else
			DEBUG_PRINTF("Sleep state\r\n");
	}
	
	
	// read raw data	
	//LSM6DS3_ACC_GYRO_R_XLDA(LSM6DS3_X_0_handle, &status);
	//if (status & LSM6DS3_ACC_GYRO_XLDA_DATA_AVAIL)
	{
		LSM6DS3_ACC_GYRO_GetRawGyroData(LSM6DS3_X_0_handle, (uint8_t*)&gyro_data);
		DEBUG_PRINTF("gyro_data_x = %d\r\n", gyro_data[0]);
		DEBUG_PRINTF("gyro_data_y = %d\r\n", gyro_data[1]);
		DEBUG_PRINTF("gyro_data_z = %d\r\n", gyro_data[2]);
	}
	
	//if (status & LSM6DS3_ACC_GYRO_GDA_DATA_AVAIL)
	{
		LSM6DS3_ACC_GYRO_GetRawAccData(LSM6DS3_X_0_handle, (uint8_t*)&acc_data);
		DEBUG_PRINTF("acc_data_x = %d\r\n", acc_data[0]);
		DEBUG_PRINTF("acc_data_y = %d\r\n", acc_data[1]);
		DEBUG_PRINTF("acc_data_z = %d\r\n", acc_data[2]);
	}
		
	
}

uint8_t Get_Accelero_State()
{
	return state;
}

void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
	if ( GPIO_Pin == M_INT1_PIN )
  {
		state = !state;
  }
	else
		if (GPIO_Pin == M_INT2_PIN)
		{
			mems_int2_detected = 1;
		}
}


