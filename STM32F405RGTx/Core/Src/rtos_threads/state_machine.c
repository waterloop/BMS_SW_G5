/*
 * state_machine.c
 *
 *  Created on: Jul. 11, 2021
 *      Author: Tiffany Wang, Ivan Mudarth
*/


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
    /*
    TODO: this function doesn't work, needs to actually
          change to the right colors, I have no idea what's
          wrong with it right now though
             - Ryan

    Assigned to: Ryan, Ivaan
    */     
    _set_ch_duty_cycle(1, B);
    _set_ch_duty_cycle(2, G);
    _set_ch_duty_cycle(3, R);
}

// Returns normal fault state or no fault based on current, voltage, and temperature measurements
State_t NormalFaultChecking(void) {
    float current = global_bms_data.battery.current;
    if (current < MIN_CURRENT_NORMAL) 
        return NormalDangerFault;
    int volt_faults = 0;
    int temp_faults = 0;
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
    return NoFault;
}

// Returns severe fault state or no fault based on current, voltage, and temperature measurements
State_t SevereFaultChecking(void) {
    float current = global_bms_data.battery.current;
    if (current > MAX_CURRENT_SEVERE) 
        return SevereDangerFault;
    int volt_faults = 0;
    int temp_faults = 0;
    for (int i = 0; i < NUM_CELLS; ++i) {
        float voltage = global_bms_data.battery.cells[i].voltage;
        float temperature = global_bms_data.battery.cells[i].temp;
        if (voltage > MAX_VOLTAGE_SEVERE || voltage < MIN_VOLTAGE_SEVERE) {
            ++volt_faults;
        } 
        if (temperature > MAX_TEMP_SEVERE) {
            ++temp_faults;
        } 
        if (volt_faults > MIN_VOLT_FAULTS || temp_faults > MIN_TEMP_FAULTS) {
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
    State_t severe_check = SevereFaultChecking();
    State_t normal_check = NormalFaultChecking();
    if (severe_check != NoFault) {
        return severe_check;
    } else if (normal_check != NoFault) {
        return normal_check;
    }

    // Resumes measurement if the previous state was Sleep
    osThreadResume(measurements_thread); 
   
    /*
    TODO: this will turn off the precharge relay EVEN IF it should be on,
          i.e., if we just finished precharging - need logic for this

    Assigned to: Ivan
    */
    TURN_OFF_PRECHARGE_PIN();
    TURN_OFF_CONTACTOR_PIN();

    // Receive CAN frame
    if (!Queue_empty(&RX_QUEUE)) {
        CANFrame rx_frame = CANBus_get_frame();
        uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
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
    State_t severe_check = SevereFaultChecking();
    State_t normal_check = NormalFaultChecking();
    if (severe_check != NoFault) {
        return severe_check;
    } else if (normal_check != NoFault) {
        return normal_check;
    }

    TURN_ON_PRECHARGE_PIN();

    // Ensure capacitors are charged
    while (global_bms_data.mc_cap_voltage < PRECHARGE_VOLTAGE_THRESHOLD) {
        osDelay(1);
    }

    /*
    TODO: need to actually send the frame by calling CANBus_put_frame,
          need to get the state_id by the LAST received state id, could probably
          store it in a global variable or something.

    Assigned to: Ivan
    */
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
    State_t severe_check = SevereFaultChecking();
    State_t normal_check = NormalFaultChecking();
    if (severe_check != NoFault) {
        return severe_check;
    } else if (normal_check != NoFault) {
        return normal_check;
    }

    /*
    TODO: need to actually send the frame by calling CANBus_put_frame,
          need to get the state_id by the LAST received state id, could probably
          store it in a global variable or something.

          Also, ACK should only be sent until AFTER the contactor is turned on.

    Assigned to: Ivan
    */
    // Send ACK on CAN when ready to run
    CANFrame rx_frame = CANBus_get_frame();
    CANFrame tx_frame = CANFrame_init(BMS_STATE_CHANGE_ACK_NACK.id);

    // TODO: state id should be from previous (idle in this case)
    uint8_t state_id = CANFrame_get_field(&rx_frame, STATE_ID);
    CANFrame_set_field(&tx_frame, BMS_STATE_CHANGE_ACK_NACK, state_id);

    if (CANBus_put_frame(&tx_frame) != HAL_OK) { Error_Handler(); }

    TURN_ON_CONTACTOR_PIN();
    TURN_OFF_PRECHARGE_PIN();

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

    /*
    TODO: need to actually send the frame by calling CANBus_put_frame,
          need to get the state_id by the LAST received state id, could probably
          store it in a global variable or something.

    Assigned to: Ivan
    */
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

void StartStateMachine(void *argument)
{
  for(;;)
  {
	OldState = CurrentState;
	CurrentState = (*SM[CurrentState].Event)();
	osDelay(200);
  }
}


