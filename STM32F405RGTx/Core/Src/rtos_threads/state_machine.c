/*
 * state_machine.c
 *
 *  Created on: Jul. 11, 2021
 *      Author: tiffanywang
 */


const osThreadAttr_t stateMachineTask_attributes = {
  .name = "stateMachineTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

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
	if (UART1_rxBuffer == "Strt") {
		return Precharging;
	} else if (UART1_rxBuffer == "Chrg") {
		return Charging;
	} else if (UART1_rxBuffer == "Stop") {
		return Sleep;
	} else {
		return Idle;
	}
	return;
}

State_t PrechargingEvent(void) {
	osDelay(3000);
	return Run;
}

State_t RunEvent(void) {
	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 1);
	// Replace pin with UART Recieve
//	if (HAL_GPIO_ReadPin(Stop_GPIO_Port, Stop_Pin)) {
//		return Stop;
//	} else {
//		return Run;
//	}
}

State_t StopEvent(void) {
	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 0);
	// Replace pin with UART Recieve
//	if (HAL_GPIO_ReadPin(Reset_GPIO_Port, Reset_Pin)) {
//		return Idle;
//	} else {
//		return Stop;
//	}
	return;
}

State_t SleepEvent(void) {
	osThreadSuspend(MeasurementsHandle); // Pauses measurements
	// Replace pin with UART Recieve
//	if (HAL_GPIO_ReadPin(Reset_GPIO_Port, Reset_Pin)) {
//		return Idle;
//	} else {
//		return Sleep;
//	}
	return;
}

State_t NormalDangerFaultEvent(void) {
	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 0);
	// Replace pin with UART Recieve
//	if (HAL_GPIO_ReadPin(Reset_GPIO_Port, Reset_Pin)) {
//		return Idle;
//	} else {
//		return NormalDangerFault;
//	}
	return;
}

State_t SevereDangerFaultEvent(void) {
	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 0);
	return SevereDangerFault;
}

State_t ChargingEvent(void) {
	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 1);
	if (BatteryPack.voltage > 51600) {
		return Charged;
	} else {
		return Charging;
	}
}

State_t ChargedEvent(void) {
	HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 0);
	// Replace pin with UART Recieve
//	if (HAL_GPIO_ReadPin(Reset_GPIO_Port, Reset_Pin)) {
//		return Idle;
//	} else {
//		return Charged;
//	}
	return;
}

State_t BalancingEvent(void) {
	return Balancing;
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartMeasurments */
/**
* @brief Function implementing the Measurments thread.
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
		HAL_UART_Transmit(&huart2, (uint8_t*)dataState, strlen(dataState), 500);
	}
	OldState = CurrentState;
	CurrentState = (*SM[CurrentState].Event)();
	osDelay(200);
  }
  /* USER CODE END StartStateMachine */
}


