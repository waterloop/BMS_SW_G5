/*
 * state_machine.c
 *
 *  Created on: Jul. 11, 2021
 *      Author: tiffanywang
*/

// TODO:
//  - Add condition for InitializeFault in Initialize event
//     - Implement LED lighting
//     - Send ACK on CAN

#include <stdio.h>
#include <string.h>
#include "state_machine.h"
#include "threads.h"
#include "cmsis_os.h"
#include "main.h"
#include "bms_entry.h"
#include "can.h"

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
    "NoFault",
    "Charging",
    "Charged",
    "Balancing"
};

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
    _set_ch_duty_cycle(1, B);
    _set_ch_duty_cycle(2, R);
    _set_ch_duty_cycle(3, G);
}

// Returns fault state or NULL based on current, voltage, and temperature measurements
// State_t FaultChecking(void *min_current, void *max_current, float max_voltage, float min_voltage, float max_temp, 
//                         float min_volt, float min_temp, State_t FaultType) {
//     float current = global_bms_data.battery.current;
//     if (min_current == NULL) {
//         float max_current_float = *( (float*)(max_current) );
//         if (current > max_current_float) 
//             return FaultType;
//     } else {
//         float min_current_float = *( (float*)(min_current) );
//         if (current < min_current_float) 
//             return FaultType;
//     }
//     int volt_faults = 0;
//     int temp_faults = 0;
//     for (int i = 0; i < NUM_CELLS; ++i) {
//         float voltage = global_bms_data.battery.cells[i].voltage;
//         float temperature = global_bms_data.battery.cells[i].temp;
//         if (voltage > max_voltage || voltage < min_voltage) {
//             ++volt_faults;
//         } 
//         if (temperature > max_temp) {
//             ++temp_faults;
//         } 
//         if (volt_faults > min_volt || temp_faults > min_temp) {
//             return FaultType;
//         }
//     }
//     return NoFault;
// }

State_t InitializeEvent(void) {
    osDelay(3000); // This is added to show it enters the initialize state for 3 seconds during testing
    return Idle;
}

State_t IdleEvent(void) {
    // Set LED colour to green
    SetLEDColour(0.0, 50.0, 0.0);
    
    // Fault checking
    // State_t severe_check = FaultChecking(NULL, (void*)&MAX_CURRENT_SEVERE, MAX_VOLTAGE_SEVERE, MIN_VOLTAGE_SEVERE, MAX_TEMP_SEVERE, 
    //                                     MIN_VOLT_FAULTS, MIN_TEMP_FAULTS, SevereDangerFault);
    // State_t normal_check = FaultChecking((void*)&MIN_CURRENT_NORMAL, NULL, MAX_VOLTAGE_NORMAL, MIN_VOLTAGE_NORMAL, MAX_TEMP_NORMAL, 
    //                                     MIN_VOLT_FAULTS, MIN_TEMP_FAULTS, NormalDangerFault);
    // if (severe_check != NoFault) {
    //     return severe_check;
    // } else if (normal_check != NoFault) {
    //     return normal_check;
    // }

    // Resumes measurement if the previous state was Sleep
    osThreadResume(measurements_thread); 
    
    TURN_OFF_PRECHARGE_PIN();
    TURN_OFF_CONTACTOR_PIN();

    // Receive CAN frame
    
    if (!Queue_empty(&RX_QUEUE)) {
        CANFrame rx_frame = CANBus_get_frame();
        uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
        if ( state_id == ARMED) {
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
    // State_t severe_check = FaultChecking(NULL, (void*)&MAX_CURRENT_SEVERE, MAX_VOLTAGE_SEVERE, MIN_VOLTAGE_SEVERE, MAX_TEMP_SEVERE, 
    //                                     MIN_VOLT_FAULTS, MIN_TEMP_FAULTS, SevereDangerFault);
    // State_t normal_check = FaultChecking((void*)&MIN_CURRENT_NORMAL, NULL, MAX_VOLTAGE_NORMAL, MIN_VOLTAGE_NORMAL, MAX_TEMP_NORMAL, 
    //                                     MIN_VOLT_FAULTS, MIN_TEMP_FAULTS, NormalDangerFault);
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

    // Send ACK on CAN 
    CANFrame rx_frame = CANBus_get_frame();
    CANFrame tx_frame = CANFrame_init(BMS_STATE_CHANGE_ACK_NACK.id);
    // TODO: state id should be from previous (idle in this case)
    uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
    CANFrame_set_field(&tx_frame, BMS_STATE_CHANGE_ACK_NACK, state_id);

    return Idle;
}

State_t RunEvent(void) {
    // Set LED colour to purple
    SetLEDColour(41.57, 5.1, 67.84);

    // Fault checking
    // State_t severe_check = FaultChecking(NULL, (void*)&MAX_CURRENT_SEVERE, MAX_VOLTAGE_SEVERE, MIN_VOLTAGE_SEVERE, MAX_TEMP_SEVERE, 
    //                                     MIN_VOLT_FAULTS, MIN_TEMP_FAULTS, SevereDangerFault);
    // State_t normal_check = FaultChecking((void*)&MIN_CURRENT_NORMAL, NULL, MAX_VOLTAGE_NORMAL, MIN_VOLTAGE_NORMAL, MAX_TEMP_NORMAL, 
    //                                     MIN_VOLT_FAULTS, MIN_TEMP_FAULTS, NormalDangerFault);
    // if (severe_check != NoFault) {
    //     return severe_check;
    // } else if (normal_check != NoFault) {
    //     return normal_check;
    // }

    // Send ACK on CAN when ready to run
    CANFrame rx_frame = CANBus_get_frame();
    CANFrame tx_frame = CANFrame_init(BMS_STATE_CHANGE_ACK_NACK.id);

    // TODO: state id should be from previous (idle in this case)
    uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
    CANFrame_set_field(&tx_frame, BMS_STATE_CHANGE_ACK_NACK, state_id);

    TURN_OFF_PRECHARGE_PIN();
    TURN_ON_CONTACTOR_PIN();

    // Receive CAN frame
    
    if (!Queue_empty(&RX_QUEUE)) {
        rx_frame = CANBus_get_frame();
        uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
        if ( state_id == BRAKING || state_id == EMERGENCY_BRAKE) {
            return Stop;
        } else {
            return Run;
        }
    }
    return Run;
}

State_t StopEvent(void) {
    // Set LED colour to blinking blue
    // TODO: change to yellow
    SetLEDColour(0.0, 0.0, 50.0);

    TURN_OFF_CONTACTOR_PIN();

    // Send ACK on CAN when stop complete
    CANFrame rx_frame = CANBus_get_frame();
    CANFrame tx_frame = CANFrame_init(BMS_STATE_CHANGE_ACK_NACK.id);
    // TODO: state id should be from previous (idle in this case)
    uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
    CANFrame_set_field(&tx_frame, BMS_STATE_CHANGE_ACK_NACK, state_id);

    // Receive CAN frame
    if (!Queue_empty(&RX_QUEUE)) {
        rx_frame = CANBus_get_frame();
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
    while (1) {
        if (!Queue_empty(&RX_QUEUE)) {
            CANFrame rx_frame = CANBus_get_frame();
            uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
            if ( state_id == RESTING ) {
                return Idle;
            } else {
                return InitializeFault;
            }
        }
    }
    return InitializeFault;
}

State_t NormalDangerFaultEvent(void) {
    // Set LED colour to red
    // TODO: change to light orange
    SetLEDColour(50.00, 0.0, 0.0);

    // Report fault on CAN
    // CANFrame tx_frame = CANFrame_init(BMS_FAULT_REPORT.id);
    // CANFrame_set_field(&tx_frame, BMS_FAULT_REPORT);
    // TODO: CAN put frame function

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
    // Set LED colour to blinking red
    // TODO: change to red
    osDelay(500);
    SetLEDColour(50.00, 0.0, 0.0);

    // Report fault on CAN
    // CANFrame tx_frame = CANFrame_init(BMS_FAULT_REPORT.id);
    // CANFrame_set_field(&tx_frame, BMS_FAULT_REPORT);
    // TODO: CAN put frame function

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
    return 0;
}

State_t BalancingEvent(void) {
    return 0;
}

State_t ChargingEvent(void) {
    return 0;
}

State_t ChargedEvent(void) {
    return 0;
}

State_t NoFaultEvent(void) {
    return 0;
}

void StartStateMachine(void *argument)
{
  for(;;)
  {
	OldState = CurrentState;
	CurrentState = (*SM[CurrentState].Event)();
	osDelay(200);
  }
}


