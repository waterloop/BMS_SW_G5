/*
 * state_machine.c
 *
*  Created on: Jul. 11, 2021
 *      Author: Tiffany Wang, Ivan Mudarth
*/

#include <stdio.h>
#include <string.h>
#include "state_machine.hpp"
#include "timer_utils.h"
#include "threads.hpp"
#include "cmsis_os.h"
#include "main.h"
#include "bms_entry.hpp"
#include "can.h"
#include "bsp.h"

RTOSThread StateMachineThread::thread;

State_t StateMachineThread::CurrentState;
State_t StateMachineThread::OldState;

uint8_t StateMachineThread::idle_state_id;
uint8_t StateMachineThread::run_state_id;
uint8_t StateMachineThread::bms_error_code;
bool StateMachineThread::has_precharged;
bool StateMachineThread::enable_fault_check;

void StateMachineThread::setState(State_t target_state) {
    CurrentState = target_state;
}

void StateMachineThread::setFaultChecking(bool val) {
    enable_fault_check = val;
}

// Returns normal fault state or no fault based on current, voltage, and temperature measurements
State_t StateMachineThread::normalFaultChecking(void) {
    if (global_bms_data.battery.current > MAX_PACK_CURRENT_NORMAL) {
        bms_error_code = BATTERY_OVERCURRENT_ERR;
        return NormalDangerFault;
    }
    if (global_bms_data.battery.voltage > MAX_PACK_VOLTAGE_NORMAL) {
        bms_error_code = BATTERY_OVERCURRENT_ERR;
        return NormalDangerFault;
    }
    if (global_bms_data.battery.voltage < MIN_PACK_VOLTAGE_NORMAL) {
        bms_error_code = BATTERY_UNDERVOLTAGE_ERR;
        return NormalDangerFault;
    }
    if (global_bms_data.buck_temp > MAX_BUCK_TEMP_NORMAL) {
        bms_error_code = BUCK_TEMPERATURE_ERR;
        return NormalDangerFault;
    }

    int overvolt_faults = 0;
    int undervolt_faults = 0;
    int temp_faults = 0;
    for (int i = 0; i < NUM_CELLS; ++i) {
        // Check if cell measurements should be flagged as a fault
        float voltage = global_bms_data.battery.cells[i].voltage;
        // float temperature = global_bms_data.battery.cells[i].temp;
        if (voltage > MAX_CELL_VOLTAGE_NORMAL) {
            ++overvolt_faults;
        }
        else if (voltage < MIN_CELL_VOLTAGE_NORMAL) {
            ++undervolt_faults;
        }

        // if (temperature > MAX_TEMP_NORMAL) {
        //     ++temp_faults;
        // } 

        // Return faults if appropriate
        if (overvolt_faults > MIN_CELL_OVERVOLT_FAULTS || undervolt_faults > MIN_CELL_UNDERVOLT_FAULTS || temp_faults > MIN_CELL_TEMP_FAULTS) {
            if (overvolt_faults > MIN_CELL_OVERVOLT_FAULTS) {
                bms_error_code = CELL_OVERVOLTAGE_ERR; 
            }
            else if (undervolt_faults > MIN_CELL_UNDERVOLT_FAULTS) {
                bms_error_code = CELL_UNDERVOLTAGE_ERR; 
            }
            else {
                bms_error_code = CELL_TEMPERATURE_ERR; 
            }            
            return NormalDangerFault;
        }
    }
    return NoFault;
}

// Returns severe fault state or no fault based on current, voltage, and temperature measurements
State_t StateMachineThread::severeFaultChecking(void) {
    if (global_bms_data.battery.current > MAX_PACK_CURRENT_SEVERE) {
        bms_error_code = BATTERY_OVERCURRENT_ERR;
        return SevereDangerFault;
    }
    if (global_bms_data.battery.voltage > MAX_PACK_VOLTAGE_SEVERE) {
        bms_error_code = BATTERY_OVERCURRENT_ERR;
        return NormalDangerFault;
    }
    if (global_bms_data.battery.voltage < MIN_PACK_VOLTAGE_SEVERE) {
        bms_error_code = BATTERY_UNDERVOLTAGE_ERR;
        return NormalDangerFault;
    }
    if (global_bms_data.buck_temp > MAX_BUCK_TEMP_SEVERE) {
        bms_error_code = BUCK_TEMPERATURE_ERR;
        return NormalDangerFault;
    }

    int overvolt_faults = 0;
    int undervolt_faults = 0;
    int temp_faults = 0;
    for (int i = 0; i < NUM_CELLS; ++i) {
        // Check if cell measurements should be flagged as a fault
        float voltage = global_bms_data.battery.cells[i].voltage;
        // float temperature = global_bms_data.battery.cells[i].temp;
        if (voltage > MAX_CELL_VOLTAGE_SEVERE) {
            ++overvolt_faults;
        }
        else if (voltage < MIN_CELL_VOLTAGE_SEVERE) {
            ++undervolt_faults;
        }
        
        // if (temperature > MAX_CELL_TEMP_SEVERE) {
        //     ++temp_faults;
        // } 

        // Return faults if appropriate
        if (overvolt_faults > MIN_CELL_OVERVOLT_FAULTS || undervolt_faults > MIN_CELL_UNDERVOLT_FAULTS || temp_faults > MIN_CELL_TEMP_FAULTS) {
            if (overvolt_faults > MIN_CELL_OVERVOLT_FAULTS) {
                bms_error_code = CELL_OVERVOLTAGE_ERR; 
            } else if (undervolt_faults > MIN_CELL_UNDERVOLT_FAULTS) {
                bms_error_code = CELL_UNDERVOLTAGE_ERR; 
            } else {
                bms_error_code = CELL_TEMPERATURE_ERR; 
            }            
            return SevereDangerFault;
        }
    }
    return NoFault;
}

State_t StateMachineThread::InitializeEvent(void) {
    return Idle;
}

State_t StateMachineThread::IdleEvent(void) {
    // Set LED colour to green
    LEDThread::setLED(0.0, 50.0, 0.0, false);

    // Resumes measurement if the previous state was Sleep
    // MeasurementsThread::resumeMeasurements(); 
   
    if (!has_precharged) {
        TURN_OFF_PRECHARGE_PIN();
    }
    TURN_OFF_CONT1_PIN();

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

State_t StateMachineThread::PrechargingEvent(void) {
    // Set LED colour to white
    LEDThread::setLED(50.0, 50.0, 50.0, false);

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
    if (send_frame(&tx_frame) != HAL_OK) { Error_Handler(); }

    return Idle;
}

State_t StateMachineThread::RunEvent(void) {
    // Set LED colour to purple
    /*
    TODO: This isn't actually purple, it's kinda blue

        Assigned to - Ivan
    */
    LEDThread::setLED(41.57, 5.1, 67.84, false);

    TURN_ON_CONT1_PIN();
    TURN_OFF_PRECHARGE_PIN();

    // Send ACK on CAN when ready to run
    CANFrame tx_frame = CANFrame_init(BMS_STATE_CHANGE_ACK_NACK);
    CANFrame_set_field(&tx_frame, STATE_CHANGE_ACK_ID, idle_state_id);
    CANFrame_set_field(&tx_frame, STATE_CHANGE_ACK, 0x00);
    if (send_frame(&tx_frame) != HAL_OK) { Error_Handler(); }

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

State_t StateMachineThread::StopEvent(void) {
    // Set LED colour to yellow
    LEDThread::setLED(50.0, 50.0, 0.0, false);

    TURN_OFF_CONT1_PIN();

    // Send ACK on CAN when stop complete
    CANFrame tx_frame = CANFrame_init(BMS_STATE_CHANGE_ACK_NACK);
    CANFrame_set_field(&tx_frame, STATE_CHANGE_ACK_ID, run_state_id);
    CANFrame_set_field(&tx_frame, STATE_CHANGE_ACK, 0x00);
    if (send_frame(&tx_frame) != HAL_OK) { Error_Handler(); }

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

State_t StateMachineThread::SleepEvent(void) {
    // Set LED colour to blue
    LEDThread::setLED(0.0, 0.0, 50.0, false);

    // Pauses measurements
    MeasurementsThread::stopMeasurements();

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


State_t StateMachineThread::InitializeFaultEvent(void) {
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

State_t StateMachineThread::NormalDangerFaultEvent(void) {
    // Set LED colour to light orange
    LEDThread::setLED(50.00, 0.0, 0.0, false);

    // Report fault on CAN
    CANFrame tx_frame = CANFrame_init(BMS_SEVERITY_CODE.id);
    CANFrame_set_field(&tx_frame, BMS_SEVERITY_CODE, DANGER);
    CANFrame_set_field(&tx_frame, BMS_ERROR_CODE, bms_error_code);
    // if (send_frame(&tx_frame) != HAL_OK) { Error_Handler(); }

    TURN_OFF_CONT1_PIN();

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

State_t StateMachineThread::SevereDangerFaultEvent(void) {
    // Set LED colour to red
    LEDThread::setLED(50.00, 0.0, 0.0, true);

    // Report fault on CAN
    CANFrame tx_frame = CANFrame_init(BMS_SEVERITY_CODE.id);
    CANFrame_set_field(&tx_frame, BMS_SEVERITY_CODE, SEVERE);
    CANFrame_set_field(&tx_frame, BMS_ERROR_CODE, bms_error_code);
    if (send_frame(&tx_frame) != HAL_OK) { Error_Handler(); }

    TURN_OFF_CONT1_PIN();

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

State_t StateMachineThread::BalancingEvent(void) {
    return Balancing;
}

State_t StateMachineThread::ChargingEvent(void) {
    return Charging;
}

State_t StateMachineThread::ChargedEvent(void) {
    return Charged;
}

State_t StateMachineThread::NoFaultEvent(void) {
    return NoFault;
}

void StateMachineThread::initialize() {
    thread = RTOSThread(
        "state_machine_thread",
        1024*3,
        STATE_MACHINE_THREAD_PRIORITY,
        runStateMachine
    );

    CurrentState = Initialize;
    has_precharged = false;
    enable_fault_check = true;
}

void StateMachineThread::runStateMachine(void *argument) {
  while(1)
  {
    // TIMING_GPIO_Port->ODR |= TIMING_Pin;
    if (enable_fault_check) {
        State_t severe_check = severeFaultChecking();
        State_t normal_check = normalFaultChecking();
        if (severe_check != NoFault) {
            CurrentState = severe_check;
        } else if (normal_check != NoFault) {
            CurrentState = normal_check;
        }
    }

	OldState = CurrentState;
    switch (CurrentState) {
        case Initialize:
            CurrentState = InitializeEvent();
            break;

        case InitializeFault:
            CurrentState = InitializeFaultEvent();
            break;

        case Idle:
            CurrentState = IdleEvent();
            break;

        case Precharging:
            CurrentState = PrechargingEvent();
            break;

        case Run:
            CurrentState = RunEvent();
            break;

        case Stop:
            CurrentState = StopEvent();
            break;

        case Sleep:
            CurrentState = SleepEvent();
            break;

        case NormalDangerFault:
            CurrentState = NormalDangerFaultEvent();
            break;

        case SevereDangerFault:
            CurrentState = SevereDangerFaultEvent();
            break;

        case NoFault:
            CurrentState = NoFaultEvent();
            break;

        case Charging:
            CurrentState = ChargingEvent();
            break;

        case Charged:
            CurrentState = ChargedEvent();
            break;

        case Balancing:
            CurrentState = BalancingEvent();
            break;

        default:
            Error_Handler();
    }

    // TIMING_GPIO_Port->ODR &= ~(TIMING_Pin);
	osDelay(STATE_MACHINE_THREAD_PERIODICITY);
  }
}


