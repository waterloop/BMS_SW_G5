/*
 * state_machine.c
 *
 *  Created on: Jul. 11, 2021
 *      Author: tiffanywang
*/

// TODO:
// 	- Add condition for Sleep state in Idle event
// 	- Impliment LED lighting
// 	- Send ACK on CAN

#include <stdio.h>
#include <string.h>
#include "state_machine.h"
#include "threads.h"
#include "cmsis_os.h"
#include "main.h"
#include "bms_entry.h"
#include "wloop_can.h"

uint8_t UART1_rxBuffer[4] = {0};

const osThreadAttr_t state_machine_thread_attrs = {
	.name = "state_machine_thread",
	.priority = (osPriority_t)osPriorityNormal,
	.stack_size = 1024*3
};

/* This is created to display the state name in serial terminal. */
const char *StateNames[] = {
	"Initialize",
	"Idle",
	"Precharging",
	"Run",
	"Stop",
	"Sleep",
	"InitializeFault",
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
	{InitializeFault, InitializeFaultEvent},
	{NormalDangerFault, NormalDangerFaultEvent},
	{SevereDangerFault, SevereDangerFaultEvent},
	{Charging, ChargingEvent},
	{Charged, ChargedEvent},
	{Balancing, BalancingEvent}
};

State_t InitializeEvent(void) {
	osDelay(3000); // This is added to show it enters the initialize state for 3 seconds during testing
	return Idle;
}

State_t IdleEvent(void) {
	osThreadResume(measurements_thread); // Resumes measurement if the previous state was Sleep
	
	TURN_OFF_PRECHARGE_PIN();
	TURN_OFF_CONTACTOR_PIN();

	// Fault checking
	float current = global_bms_data.battery.current;
	if (current > MAX_CURRENT_SEVERE) {
		return SevereDangerFault;
	} 
	volt_faults = 0;
	temp_faults = 0;
	for (int i = 0; i < NUM_CELLS; ++i) {
		float voltage = global_bms_data.battery.cells[i].voltage;
		float temperature = global_bms_data.battery.cells[i].temp;
		if (voltage > MAX_VOLTAGE_SEVERE || voltage < MIN_VOLTAGE_SEVERE {
			++volt_faults;
		} 
		if (temperature > MAX_TEMP_SEVERE) {
			++temp_faults;
		} 
		if (volt_faults > MIN_VOLT_FAULTS || temp_faults > MIN_TEMP_FAULTS) {
			return SevereDangerFault;
		}
	}
	// Receive CAN frame
	if ( CANBus_init(&hcan1) != HAL_OK) { Error_Handler(); }
	CANFrame rx_frame = CANBus_get_frame();
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	if ( state_id == ARMED) {
		return Precharging;
	} else if (state_id == AUTO_PILOT) {
		return Run;
	} 
}

State_t PrechargingEvent(void) {
	TURN_ON_PRECHARGE_PIN();
	// Ensure capacitors are charged
	while (global_bms_data.mc_cap_voltage < PRECHARGE_VOLTAGE_THRESHOLD) {
		osDelay(1);
	}
	return Idle;
}

State_t RunEvent(void) {
	TURN_OFF_PRECHARGE_PIN();
	TURN_ON_CONTACTOR_PIN();

	// Fault checking
	float current = global_bms_data.battery.current;
	if (current < MIN_CURRENT_NORMAL) {
		return NormalDangerFault;
	}
	volt_faults = 0;
	temp_faults = 0;
	for (int i = 0; i < NUM_CELLS; ++i) {
		float voltage = global_bms_data.battery.cells[i].voltage;
		float temperature = global_bms_data.battery.cells[i].temp;
		if (voltage > MAX_VOLTAGE_NORMAL || voltage < MIN_VOLTAGE_NORMAL) {
			++volt_faults;
		} 
		if (temperature > MAX_TEMP_NORMAL) {
			++temp_faults;
		}
		if (volt_faults > MIN_VOLT_FAULTS || temp_faults > MIN_TEMP_FAULTS) {
			return NormalDangerFault;
		}
	}
	// Receive CAN frame
	if ( CANBus_init(&hcan1) != HAL_OK) { Error_Handler(); }
	CANFrame rx_frame = CANBus_get_frame();
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	if ( state_id == BRAKING || state_id == EMERGENCY_BRAKE) {
		return Stop;
	} else {
		return Run;
	} 
}

State_t StopEvent(void) {
	TURN_OFF_CONTACTOR_PIN();
	// Receive CAN frame
	if ( CANBus_init(&hcan1) != HAL_OK) { Error_Handler(); }
	CANFrame rx_frame = CANBus_get_frame();
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	if ( state_id == RESTING) {
		return Idle;
	} else {
		return Stop;
	} 
}

State_t SleepEvent(void) {
	osThreadSuspend(measurements_thread); // Pauses measurements
	// Receive CAN frame
	if ( CANBus_init(&hcan1) != HAL_OK) { Error_Handler(); }
	CANFrame rx_frame = CANBus_get_frame();
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	if ( state_id == RESTING) {
		return Idle;
	} else {
		return Sleep;
	} 
}


State_t InitializeFaultEvent(void) {
	// Receive CAN frame
	if ( CANBus_init(&hcan1) != HAL_OK) { Error_Handler(); }
	CANFrame rx_frame = CANBus_get_frame();
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	if ( state_id == RESTING ) {
		return Idle;
	} else {
		return InitializeFault;
	}
}

State_t NormalDangerFaultEvent(void) {
	TURN_OFF_CONTACTOR_PIN();
	// Receive CAN frame
	if ( CANBus_init(&hcan1) != HAL_OK) { Error_Handler(); }
	CANFrame rx_frame = CANBus_get_frame();
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	if ( state_id == RESTING ) {
		return Idle;
	} else {
		return NormalDangerFault;
	}
}

State_t SevereDangerFaultEvent(void) {
	TURN_OFF_CONTACTOR_PIN();
	// Receive CAN frame
	if ( CANBus_init(&hcan1) != HAL_OK) { Error_Handler(); }
	CANFrame rx_frame = CANBus_get_frame();
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	if ( state_id == RESTING ) {
		return Idle;
	} else {
		return SevereDangerFault;
	}
}

// Not implemented yet
State_t BalancingEvent(void) {
	// Receive CAN frame
	if ( CANBus_init(&hcan1) != HAL_OK) { Error_Handler(); }
	CANFrame rx_frame = CANBus_get_frame();
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	if ( state_id == RESTING ) {
		return Idle;
	} else {
		return Balancing;
	}
}

void StartStateMachine(void *argument)
{
  for(;;)
  {
	// Print CurrentState in serial terminal if the state changes
	if (OldState != CurrentState) {
		char dataState[100];
		sprintf(dataState, "Current State: %s\r\n", StateNames[CurrentState]);
		HAL_UART_Transmit(&huart1, (uint8_t*)dataState, strlen(dataState), 500); // Implement using CAN
	}
	OldState = CurrentState;
	CurrentState = (*SM[CurrentState].Event)();
	osDelay(200);
  }
}


