/*
 * state_machine.c
 *
 *  Created on: Jul. 11, 2021
 *      Author: tiffanywang
 */

#include <stdio.h>
#include <string.h>
#include "state_machine.h"
#include "cmsis_os.h"
#include "main.h"

uint8_t UART1_rxBuffer[4] = {0};


/* Definitions for Measurements */
osThreadId_t MeasurementsHandle;
const osThreadAttr_t Measurements_attributes = {
  .name = "Measurements",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* creation of Measurements */
// MeasurementsHandle = osThreadNew(StartMeasurments, NULL, &Measurements_attributes);


/* This is created to display the state name in serial terminal. */
const char *StateNames[] = {
	"Initialize",
	"Idle",
	"Precharging",
	"Run",
	"Stop",
	"Sleep",
	"NormalDangerFault",
	"SevereDangerFault",
	"Charging",
	"Charged",
	"Balancing"
};

State_t CurrentState = Initialize;
State_t OldState = Sleep;

StateMachine SM[11] = {
    {Initialize, InitializeEvent},
	{Idle, IdleEvent},
	{Precharging, PrechargingEvent},
	{Run, RunEvent},
	{Stop, StopEvent},
	{Sleep, SleepEvent},
	{NormalDangerFault, NormalDangerFaultEvent},
	{SevereDangerFault, SevereDangerFaultEvent},
	{Charging, ChargingEvent},
	{Charged, ChargedEvent},
	{Balancing, BalancingEvent}
};



/* USER CODE BEGIN 4 */
/* Defining the conditions necessary for state transitions */

State_t InitializeEvent(void) {
	osDelay(3000); // This is added to show it enters the initialize state for 3 seconds during testing
	return Idle;
}

State_t IdleEvent(void) {
	osThreadResume(MeasurementsHandle); // Resumes measurement if the previous state was Sleep
	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 0);
	HAL_UART_Receive (&huart1, UART1_rxBuffer, 4, 5000);
	if (strcmp( (char*)UART1_rxBuffer, "Strt" ) == 0) {
		return Precharging;
	} else if (strcmp( (char*)UART1_rxBuffer, "Chrg" ) == 0) {
		return Charging;
	} else if (strcmp( (char*)UART1_rxBuffer, "Stop") == 0) {
		return Sleep;
	} else {
		return Idle;
	}
}

State_t PrechargingEvent(void) {
	HAL_GPIO_WritePin(PRECHARGE_GPIO_Port, PRECHARGE_Pin, 1);
	osDelay(3000);
	return Run;
}

State_t RunEvent(void) {
	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 1);
	HAL_GPIO_WritePin(PRECHARGE_GPIO_Port, PRECHARGE_Pin, 0);
	// Replace pin with UART Receive
	if (strcmp( (char*)UART1_rxBuffer, "Stop" ) == 0) {
		return Stop;
	} else {
		return Run;
	}
}

State_t StopEvent(void) {
	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 0);
	// Replace pin with UART Receive
	if (strcmp( (char*)UART1_rxBuffer, "Rset") == 0) {
		return Idle;
	} else {
		return Stop;
	}
}

State_t SleepEvent(void) {
	osThreadSuspend(MeasurementsHandle); // Pauses measurements
	// Replace pin with UART Receive
	if (strcmp( (char*)UART1_rxBuffer, "Rset") == 0) {
		return Idle;
	} else {
		return Sleep;
	}
}

State_t NormalDangerFaultEvent(void) {
	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 0);
	// Replace pin with UART Receive
	if (strcmp( (char*)UART1_rxBuffer, "Rset") == 0) {
		return Idle;
} else {
		return NormalDangerFault;
	}
}

State_t SevereDangerFaultEvent(void) {
	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 0);
	return SevereDangerFault;
}

State_t ChargingEvent(void) {
	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 1);
	if (global_bms_data.battery.voltage > 51600) {
		return Charged;
	} else {
		return Charging;
	}
}

State_t ChargedEvent(void) {
	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 0);
	// Replace pin with UART Receive
	if (strcmp( (char*)UART1_rxBuffer, "Rset") == 0) {
		return Idle;
} else {
		return Charged;
	}
}

State_t BalancingEvent(void) {
	return Balancing;
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartMeasurments */
/**
* @brief Function implementing the Measurements thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMeasurments */
void StartMeasurments(void *argument)
{
  /* USER CODE BEGIN StartMeasurments */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartMeasurments */
}



/* USER CODE BEGIN Header_StartStateMachine */
/**
* @brief Function implementing the StateMachine thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartStateMachine */
void StartStateMachine(void *argument)
{
  /* USER CODE BEGIN StartStateMachine */
  /* Infinite loop */
  for(;;)
  {
	// Print CurrentState in serial terminal if the state changes
	if (OldState != CurrentState) {
		char dataState[100];
		sprintf(dataState, "Current State: %s\r\n", StateNames[CurrentState]);
		HAL_UART_Transmit(&huart1, (uint8_t*)dataState, strlen(dataState), 500);
	}
	OldState = CurrentState;
	CurrentState = (*SM[CurrentState].Event)();
	osDelay(200);
  }
  /* USER CODE END StartStateMachine */
}


