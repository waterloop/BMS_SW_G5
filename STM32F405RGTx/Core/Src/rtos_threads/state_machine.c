/*
 * state_machine.c
 *
 *  Created on: Jul. 11, 2021
 *      Author: tiffanywang
*/

// TODO:
//  - Add condition for InitializeFault in Initialize event
// 	- Implement LED lighting
// 	- Send ACK on CAN

#include <stdio.h>
#include <string.h>
#include "state_machine.h"
#include "threads.h"
#include "cmsis_os.h"
#include "main.h"
#include "bms_entry.h"
#include "wloop_can.h"

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

// Set channel duty cycle
void _set_ch_duty_cycle(uint8_t ch, float dc) {
	uint32_t ccr_val = (uint32_t)( ((100 - dc)*ARR_VAL)/100 );
	switch (ch) {
		case 1:
			htim1.Instance->CCR1 = ccr_val;
		case 2:
			htim1.Instance->CCR2 = ccr_val;
		case 3:
			htim1.Instance->CCR3 = ccr_val;
		case 4:
			htim1.Instance->CCR4 = ccr_val;
	}
}

// Set LED colour based on channel duty cycles for RGB channels
void SetLEDColour(float R, float G, float B) {
	htim1.Instance->CCR1 = ccr_val = R;
	htim1.Instance->CCR2 = ccr_val = G;
	htim1.Instance->CCR3 = ccr_val = B;	
}

// Returns fault state or NULL based on current, voltage, and temperature measurements
State_t FaultChecking(float min_current, float max_current, float max_voltage, float min_voltage, float max_temp, 
						float min_volt, float min_temp, State_t FaultType) {
	float current = global_bms_data.battery.current;
	if (min_current == NULL) {
		if (current > max_current) 
			return FaultType;
	} else {
		if (current < min_current) 
			return FaultType;
	}
	volt_faults = 0;
	temp_faults = 0;
	for (int i = 0; i < NUM_CELLS; ++i) {
		float voltage = global_bms_data.battery.cells[i].voltage;
		float temperature = global_bms_data.battery.cells[i].temp;
		if (voltage > max_voltage || voltage < min_voltage) {
			++volt_faults;
		} 
		if (temperature > max_temp) {
			++temp_faults;
		} 
		if (volt_faults > min_volt || temp_faults > min_temp) {
			return FaultType;
		}
	}
	return NULL;
}

State_t InitializeEvent(void) {
	osDelay(3000); // This is added to show it enters the initialize state for 3 seconds during testing
	return Idle;
}

State_t IdleEvent(void) {
	// Set LED colour to green
	SetLEDColour(0.0, 100.0, 0.0);
	
	// Fault checking
	State_t severe_check = FaultChecking(NULL, MAX_CURRENT_SEVERE, MAX_VOLTAGE_SEVERE, MIN_VOLTAGE_SEVERE, MAX_TEMP_SEVERE, 
										MIN_VOLT_FAULTS, MIN_TEMP_FAULTS, SevereDangerFault);
	State_t normal_check = FaultChecking(MIN_CURRENT_NORMAL, NULL, MAX_VOLTAGE_NORMAL, MIN_VOLTAGE_NORMAL, MAX_TEMP_NORMAL, 
										MIN_VOLT_FAULTS, MIN_TEMP_FAULTS, NormalDangerFault);
	if (severe_check != NULL) {
		return severe_check;
	} else if (normal_check != NULL) {
		return normal_check;
	}

	// Resumes measurement if the previous state was Sleep
	osThreadResume(measurements_thread); 
	
	TURN_OFF_PRECHARGE_PIN();
	TURN_OFF_CONTACTOR_PIN();

	// Receive CAN frame
	CANFrame rx_frame = CANBus_get_frame();
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	if ( state_id == ARMED) {
		return Precharging;
	} else if (state_id == AUTO_PILOT) {
		return Run;
	} 
}

State_t PrechargingEvent(void) {
	// Set LED colour to white
	SetLEDColour(100.0, 100.0, 100.0);

	// Fault checking
	State_t severe_check = FaultChecking(NULL, MAX_CURRENT_SEVERE, MAX_VOLTAGE_SEVERE, MIN_VOLTAGE_SEVERE, MAX_TEMP_SEVERE, 
										MIN_VOLT_FAULTS, MIN_TEMP_FAULTS, SevereDangerFault);
	State_t normal_check = FaultChecking(MIN_CURRENT_NORMAL, NULL, MAX_VOLTAGE_NORMAL, MIN_VOLTAGE_NORMAL, MAX_TEMP_NORMAL, 
										MIN_VOLT_FAULTS, MIN_TEMP_FAULTS, NormalDangerFault);
	if (severe_check != NULL) {
		return severe_check;
	} else if (normal_check != NULL) {
		return normal_check;
	}

	TURN_ON_PRECHARGE_PIN();

	// Ensure capacitors are charged
	while (global_bms_data.mc_cap_voltage < PRECHARGE_VOLTAGE_THRESHOLD) {
		osDelay(1);
	}

	// Send ACK on CAN 
	CANFrame rx_frame = CANBus_get_frame();
	CANFrame tx_frame = CANFrame_init(BMS_STATE_CHANGE_ACK_NACK.id);
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	CANFrame_set_field(&tx_frame, BMS_STATE_CHANGE_ACK_NACK, state_id);

	return Idle;
}

State_t RunEvent(void) {
	// Set LED colour to purple
	SetLEDColour(41.57, 5.1, 67.84);

	// Fault checking
	State_t severe_check = FaultChecking(NULL, MAX_CURRENT_SEVERE, MAX_VOLTAGE_SEVERE, MIN_VOLTAGE_SEVERE, MAX_TEMP_SEVERE, 
										MIN_VOLT_FAULTS, MIN_TEMP_FAULTS, SevereDangerFault);
	State_t normal_check = FaultChecking(MIN_CURRENT_NORMAL, NULL, MAX_VOLTAGE_NORMAL, MIN_VOLTAGE_NORMAL, MAX_TEMP_NORMAL, 
										MIN_VOLT_FAULTS, MIN_TEMP_FAULTS, NormalDangerFault);
	if (severe_check != NULL) {
		return severe_check;
	} else if (normal_check != NULL) {
		return normal_check;
	}

	// Send ACK on CAN when ready to run
	CANFrame rx_frame = CANBus_get_frame();
	CANFrame tx_frame = CANFrame_init(BMS_STATE_CHANGE_ACK_NACK.id);
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	CANFrame_set_field(&tx_frame, BMS_STATE_CHANGE_ACK_NACK, state_id);

	TURN_OFF_PRECHARGE_PIN();
	TURN_ON_CONTACTOR_PIN();

	// Receive CAN frame
	rx_frame = CANBus_get_frame();
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	if ( state_id == BRAKING || state_id == EMERGENCY_BRAKE) {
		return Stop;
	} else {
		return Run;
	} 
}

State_t StopEvent(void) {
	// Set LED colour to blinking blue
	osDelay(500);
	SetLEDColour(0.0, 0.0, 100.0);

	TURN_OFF_CONTACTOR_PIN();

	// Send ACK on CAN when stop complete
	CANFrame rx_frame = CANBus_get_frame();
	CANFrame tx_frame = CANFrame_init(BMS_STATE_CHANGE_ACK_NACK.id);
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	CANFrame_set_field(&tx_frame, BMS_STATE_CHANGE_ACK_NACK, state_id);

	// Receive CAN frame
	rx_frame = CANBus_get_frame();
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	if ( state_id == RESTING) {
		return Idle;
	} else {
		return Stop;
	} 
}

State_t SleepEvent(void) {
	// Set LED colour to blue
	SetLEDColour(0.0, 0.0, 100.0);

	// Pauses measurements
	osThreadSuspend(measurements_thread); 

	// Receive CAN frame
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
	CANFrame rx_frame = CANBus_get_frame();
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	if ( state_id == RESTING ) {
		return Idle;
	} else {
		return InitializeFault;
	}
}

State_t NormalDangerFaultEvent(void) {
	// Set LED colour to red
	SetLEDColour(100.00, 0.0, 0.0);

	// Report fault on CAN
	CANFrame tx_frame = CANFrame_init(BMS_FAULT_REPORT.id);
	CANFrame_set_field(&tx_frame, BMS_FAULT_REPORT);

	TURN_OFF_CONTACTOR_PIN();

	// Receive CAN frame
	rx_frame = CANBus_get_frame();
	uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
	if ( state_id == RESTING ) {
		return Idle;
	} else {
		return NormalDangerFault;
	}
}

State_t SevereDangerFaultEvent(void) {
	// Set LED colour to blinking red
	osDelay(500);
	SetLEDColour(100.00, 0.0, 0.0);

	// Report fault on CAN
	CANFrame tx_frame = CANFrame_init(BMS_FAULT_REPORT.id);
	CANFrame_set_field(&tx_frame, BMS_FAULT_REPORT);

	TURN_OFF_CONTACTOR_PIN();

	// Receive CAN frame
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


