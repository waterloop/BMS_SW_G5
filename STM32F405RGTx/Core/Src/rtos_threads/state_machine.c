/*
 * state_machine.c
 *
 *  Created on: Jul. 11, 2021
 *      Author: Tiffany Wang, Ivan Mudarth
*/


#include <stdio.h>
#include <string.h>
#include "state_machine.h"
#include "timer_utils.h"
#include "threads.h"
#include "cmsis_os.h"
#include "main.h"
#include "bms_entry.h"
#include "can.h"

/*
Don’t check for undercurrent
Check for overvoltage 
Don't return battery overvoltage when n cells are overvoltage
    return cell overvoltage
return cell temp faults
*/
const osThreadAttr_t state_machine_thread_attrs = {
    .name = "state_machine_thread",
    .priority = (osPriority_t)osPriorityNormal,
    .stack_size = 1024*3
};

typedef enum { false = 0, true = !false } bool;

uint8_t idle_state_id;
uint8_t run_state_id;
uint8_t bms_error_code;
bool has_precharged = false;

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
    "NoFault",
    "Charging",
    "Charged",
    "Balancing"
};
// test
State_t CurrentState = Initialize;
State_t OldState = Sleep;

StateMachine SM[13] = {
    {Initialize, InitializeEvent},
    {InitializeFault, InitializeFaultEvent},
    {Idle, IdleEvent},
    {Precharging, PrechargingEvent},
    {Run, RunEvent},
    {Stop, StopEvent},
    {Sleep, SleepEvent},
    {NormalDangerFault, NormalDangerFaultEvent},
    {SevereDangerFault, SevereDangerFaultEvent},
    {NoFault, NoFaultEvent},
    {Charging, ChargingEvent},
    {Charged, ChargedEvent},
    {Balancing, BalancingEvent}
};

// Set LED colour based on channel duty cycles for RGB channels
void SetLEDColour(float R, float G, float B) {
    set_led_intensity(RED, R);
    set_led_intensity(GREEN, G);
    set_led_intensity(BLUE, B);
}

// CAN heartbeat subroutine
void SendCANHeartbeat(void) {
    float avg_cell_temp = 0;
    for (uint8_t i = 0; i < NUM_CELLS; i++) {
        avg_cell_temp += global_bms_data.battery.cells[i].temp;
    }
    avg_cell_temp /= NUM_CELLS;

    CANFrame tx_frame0 = CANFrame_init(BATTERY_PACK_CURRENT.id);
    CANFrame_set_field(&tx_frame0, BATTERY_PACK_CURRENT, FLOAT_TO_UINT(global_bms_data.battery.current));
    CANFrame_set_field(&tx_frame0, CELL_TEMPERATURE, FLOAT_TO_UINT(avg_cell_temp));

    CANFrame tx_frame1 = CANFrame_init(BATTERY_PACK_VOLTAGE.id);
    CANFrame_set_field(&tx_frame1, BATTERY_PACK_VOLTAGE, FLOAT_TO_UINT(global_bms_data.battery.voltage));
    CANFrame_set_field(&tx_frame1, STATE_OF_CHARGE, FLOAT_TO_UINT(global_bms_data.battery.soc));

    CANFrame tx_frame2 = CANFrame_init(BUCK_TEMPERATURE.id);
    CANFrame_set_field(&tx_frame2, BUCK_TEMPERATURE, FLOAT_TO_UINT(global_bms_data.buck_temp));
    // CANFrame_set_field(&tx_frame2, BMS_CURRENT, FLOAT_TO_UINT(0));

    CANFrame tx_frame3 = CANFrame_init(MC_CAP_VOLTAGE.id);
    CANFrame_set_field(&tx_frame3, MC_CAP_VOLTAGE, FLOAT_TO_UINT(global_bms_data.mc_cap_voltage));

    if (CANBus_put_frame(&tx_frame0) != HAL_OK) { Error_Handler(); }
    if (CANBus_put_frame(&tx_frame1) != HAL_OK) { Error_Handler(); }
    if (CANBus_put_frame(&tx_frame2) != HAL_OK) { Error_Handler(); }
    if (CANBus_put_frame(&tx_frame3) != HAL_OK) { Error_Handler(); }
}

// Returns normal fault state or no fault based on current, voltage, and temperature measurements
State_t NormalFaultChecking(void) {
    float current = global_bms_data.battery.current;
    if (current > MAX_CURRENT_NORMAL) {
        bms_error_code = BATTERY_OVERCURRENT_ERR;
        return NormalDangerFault;
    }
    int overvolt_faults = 0;
    int undervolt_faults = 0;
    int temp_faults = 0;
    for (int i = 0; i < NUM_CELLS; ++i) {
        // Check if cell measurements should be flagged as a fault
        float voltage = global_bms_data.battery.cells[i].voltage;
        float temperature = global_bms_data.battery.cells[i].temp;
        if (voltage > MAX_VOLTAGE_NORMAL) {
            ++overvolt_faults;
        }
        else if (voltage < MIN_VOLTAGE_NORMAL) {
            ++undervolt_faults;
        }

        if (temperature > MAX_TEMP_NORMAL) {
            ++temp_faults;
        } 
        // Return faults if appropriate
        if (overvolt_faults > MIN_OVERVOLT_FAULTS || undervolt_faults > MIN_UNDERVOLT_FAULTS || temp_faults > MIN_TEMP_FAULTS) {
            if (overvolt_faults > MIN_OVERVOLT_FAULTS) {
                bms_error_code = CELL_OVERVOLTAGE_ERR; 
            } else if (undervolt_faults > MIN_UNDERVOLT_FAULTS) {
                bms_error_code = CELL_UNDERVOLTAGE_ERR; 
            } else {
                bms_error_code = CELL_TEMPERATURE_ERR; 
            }            
            return NormalDangerFault;
        }
    }
    return NoFault;
}

// Returns severe fault state or no fault based on current, voltage, and temperature measurements
State_t SevereFaultChecking(void) {
    float current = global_bms_data.battery.current;
    if (current > MAX_CURRENT_SEVERE) {
        bms_error_code = BATTERY_OVERCURRENT_ERR;
        return SevereDangerFault;
    }
    int overvolt_faults = 0;
    int undervolt_faults = 0;
    int temp_faults = 0;
    for (int i = 0; i < NUM_CELLS; ++i) {
        // Check if cell measurements should be flagged as a fault
        float voltage = global_bms_data.battery.cells[i].voltage;
        float temperature = global_bms_data.battery.cells[i].temp;
        if (voltage > MAX_VOLTAGE_SEVERE) {
            ++overvolt_faults;
        } else if (voltage < MIN_VOLTAGE_SEVERE) {
            ++undervolt_faults;
        }
        if (temperature > MAX_TEMP_SEVERE) {
            ++temp_faults;
        } 
        // Return faults if appropriate
        if (overvolt_faults > MIN_OVERVOLT_FAULTS || undervolt_faults > MIN_UNDERVOLT_FAULTS || temp_faults > MIN_TEMP_FAULTS) {
            if (overvolt_faults > MIN_OVERVOLT_FAULTS) {
                bms_error_code = CELL_OVERVOLTAGE_ERR; 
            } else if (undervolt_faults > MIN_UNDERVOLT_FAULTS) {
                bms_error_code = CELL_UNDERVOLTAGE_ERR; 
            } else {
                bms_error_code = CELL_TEMPERATURE_ERR; 
            }            
            return SevereDangerFault;
        }
    }
    return NoFault;
}

State_t InitializeEvent(void) {
    return Idle;
}

State_t IdleEvent(void) {
    // Set LED colour to green
    SetLEDColour(0.0, 50.0, 0.0);
    
    // Fault checking
    // State_t severe_check = SevereFaultChecking();
    // State_t normal_check = NormalFaultChecking();
    // if (severe_check != NoFault) {
    //     return severe_check;
    // } else if (normal_check != NoFault) {
    //     return normal_check;
    // }

    // Resumes measurement if the previous state was Sleep
    // osThreadResume(measurements_thread); 
   
    if (!has_precharged) {
        TURN_OFF_PRECHARGE_PIN();
    }
    TURN_OFF_CONTACTOR_PIN();

    // Receive CAN frame
    if (!Queue_empty(&RX_QUEUE)) {
        CANFrame rx_frame = CANBus_get_frame();
        uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
        idle_state_id = state_id;
        if (state_id == ARMED) {
            return Precharging;
        } else if (state_id == AUTO_PILOT) {
            return Run;
        }
    }

    return Idle;
}

State_t PrechargingEvent(void) {
    // Set LED colour to white
    SetLEDColour(50.0, 50.0, 50.0);

    // Fault checking
    // State_t severe_check = SevereFaultChecking();
    // State_t normal_check = NormalFaultChecking();
    // if (severe_check != NoFault) {
    //     return severe_check;
    // } else if (normal_check != NoFault) {
    //     return normal_check;
    // }

    TURN_ON_PRECHARGE_PIN();

    // Ensure capacitors are charged
    while (global_bms_data.mc_cap_voltage < PRECHARGE_VOLTAGE_THRESHOLD) {
        osDelay(1);
    }
    has_precharged = true;

    // Send ACK on CAN 
    CANFrame tx_frame = CANFrame_init(BMS_STATE_CHANGE_ACK_NACK);
    CANFrame_set_field(&tx_frame, STATE_CHANGE_ACK_ID, idle_state_id);
    CANFrame_set_field(&tx_frame, STATE_CHANGE_ACK, 0x00);
    if (CANBus_put_frame(&tx_frame) != HAL_OK) { Error_Handler(); }

    return Idle;
}

State_t RunEvent(void) {
    // Set LED colour to purple
    /*
    TODO: This isn't actually purple, it's kinda blue

        Assigned to - Ivan
    */
    SetLEDColour(41.57, 5.1, 67.84);

    // Fault checking
    // State_t severe_check = SevereFaultChecking();
    // State_t normal_check = NormalFaultChecking();
    // if (severe_check != NoFault) {
    //     return severe_check;
    // } else if (normal_check != NoFault) {
    //     return normal_check;
    // }

    TURN_ON_CONTACTOR_PIN();
    TURN_OFF_PRECHARGE_PIN();

    // Send ACK on CAN when ready to run
    CANFrame tx_frame = CANFrame_init(BMS_STATE_CHANGE_ACK_NACK);
    CANFrame_set_field(&tx_frame, STATE_CHANGE_ACK_ID, idle_state_id);
    CANFrame_set_field(&tx_frame, STATE_CHANGE_ACK, 0x00);
    if (CANBus_put_frame(&tx_frame) != HAL_OK) { Error_Handler(); }

    // Receive CAN frame
    if (!Queue_empty(&RX_QUEUE)) {
        CANFrame rx_frame = CANBus_get_frame();
        uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
        run_state_id = state_id;
        if ( state_id == BRAKING || state_id == EMERGENCY_BRAKE) {
            return Stop;
        } else {
            return Run;
        }
    }
    return Run;
}

State_t StopEvent(void) {
    // Set LED colour to yellow
    SetLEDColour(50.0, 50.0, 0.0);

    TURN_OFF_CONTACTOR_PIN();

    // Send ACK on CAN when stop complete
    CANFrame tx_frame = CANFrame_init(BMS_STATE_CHANGE_ACK_NACK);
    CANFrame_set_field(&tx_frame, STATE_CHANGE_ACK_ID, run_state_id);
    CANFrame_set_field(&tx_frame, STATE_CHANGE_ACK, 0x00);
    if (CANBus_put_frame(&tx_frame) != HAL_OK) { Error_Handler(); }

    // Receive CAN frame
    if (!Queue_empty(&RX_QUEUE)) {
        CANFrame rx_frame = CANBus_get_frame();
        uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
        if ( state_id == RESTING) {
            return Idle;
        } else {
            return Stop;
        }
    }
    return Stop;
}

State_t SleepEvent(void) {
    // Set LED colour to blue
    SetLEDColour(0.0, 0.0, 50.0);

    // Pauses measurements
    osThreadSuspend(measurements_thread); 

    // Receive CAN frame
    if (!Queue_empty(&RX_QUEUE)) {
        CANFrame rx_frame = CANBus_get_frame();
        uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
        if ( state_id == RESTING) {
            return Idle;
        } else {
            return Sleep;
        }
    }
    return Sleep;
}


State_t InitializeFaultEvent(void) {
    // Receive CAN frame
    if (!Queue_empty(&RX_QUEUE)) {
        CANFrame rx_frame = CANBus_get_frame();
        uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
        if ( state_id == RESTING ) {
            return Idle;
        } else {
            return InitializeFault;
        }
    }
    return InitializeFault;
}

State_t NormalDangerFaultEvent(void) {
    // Set LED colour to light orange
    SetLEDColour(50.00, 32.3, 0.0);

    // Report fault on CAN
    CANFrame tx_frame = CANFrame_init(BMS_SEVERITY_CODE.id);
    CANFrame_set_field(&tx_frame, BMS_SEVERITY_CODE, DANGER);
    CANFrame_set_field(&tx_frame, BMS_ERROR_CODE, bms_error_code);
    if (CANBus_put_frame(&tx_frame) != HAL_OK) { Error_Handler(); }

    TURN_OFF_CONTACTOR_PIN();

    // Receive CAN frame
    if (!Queue_empty(&RX_QUEUE)) {
        CANFrame rx_frame = CANBus_get_frame();
        uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
        if ( state_id == RESTING ) {
            return Idle;
        } else {
            return NormalDangerFault;
        }
    }
    return NormalDangerFault;
}

State_t SevereDangerFaultEvent(void) {
    // Set LED colour to red
    SetLEDColour(50.00, 0.0, 0.0);

    // Report fault on CAN
    CANFrame tx_frame = CANFrame_init(BMS_SEVERITY_CODE.id);
    CANFrame_set_field(&tx_frame, BMS_SEVERITY_CODE, SEVERE);
    CANFrame_set_field(&tx_frame, BMS_ERROR_CODE, bms_error_code);
    if (CANBus_put_frame(&tx_frame) != HAL_OK) { Error_Handler(); }

    TURN_OFF_CONTACTOR_PIN();

    // Receive CAN frame
    if (!Queue_empty(&RX_QUEUE)) { 
        CANFrame rx_frame = CANBus_get_frame();
        uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
        if ( state_id == RESTING ) {
            return Idle;
        } else {
            return SevereDangerFault;
        }
    }
    return SevereDangerFault;
}

State_t BalancingEvent(void) {
    return Balancing;
}

State_t ChargingEvent(void) {
    return Charging;
}

State_t ChargedEvent(void) {
    return Charged;
}

State_t NoFaultEvent(void) {
    return NoFault;
}

void StartStateMachine(void *argument) {
  for(;;)
  {
	OldState = CurrentState;
	CurrentState = (*SM[CurrentState].Event)();
    SendCANHeartbeat();
	osDelay(200);
  }
}


