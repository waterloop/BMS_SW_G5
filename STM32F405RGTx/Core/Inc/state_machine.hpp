/*
 * state_machine.hpp
 *
 *    Created on: Jul. 11, 2021
 *            Author: tiffanywang
 */

#pragma once

#include "can.h"
#include "cmsis_os.h"
#include "can.h"
#include "util.hpp"

#define TURN_ON_PRECHARGE_PIN() (HAL_GPIO_WritePin(PRECHARGE_GPIO_Port, PRECHARGE_Pin, (GPIO_PinState)1))
#define TURN_OFF_PRECHARGE_PIN() (HAL_GPIO_WritePin(PRECHARGE_GPIO_Port, PRECHARGE_Pin, (GPIO_PinState)0))
#define TURN_ON_CONTACTOR_PIN() (HAL_GPIO_WritePin(CONT1_GPIO_Port, CONT1_Pin, (GPIO_PinState)1))
#define TURN_OFF_CONTACTOR_PIN() (HAL_GPIO_WritePin(CONT1_GPIO_Port, CONT1_Pin, (GPIO_PinState)0))

//////////////////////////////////////////////////////////////
// PACK PARAMETERS
#define PRECHARGE_VOLTAGE_THRESHOLD 40.0

#define MAX_PACK_CURRENT_SEVERE 50.0
#define MAX_PACK_CURRENT_NORMAL 40.0

#define MAX_PACK_VOLTAGE_SEVERE 50.0
#define MIN_PACK_VOLTAGE_SEVERE 40.0
#define MAX_PACK_VOLTAGE_NORMAL 47.0
#define MIN_PACK_VOLTAGE_NORMAL 44.0

#define MAX_BUCK_TEMP_SEVERE 100.0
#define MAX_BUCK_TEMP_NORMAL 80.0
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// CELL PARAMETERS
#define MAX_CELL_VOLTAGE_SEVERE 3.7
#define MIN_CELL_VOLTAGE_SEVERE 1.6
#define MAX_CELL_VOLTAGE_NORMAL 3.6
#define MIN_CELL_VOLTAGE_NORMAL 2.6

#define MAX_CELL_TEMP_SEVERE 60.0   // not final

#define MIN_CELL_OVERVOLT_FAULTS 1
#define MIN_CELL_UNDERVOLT_FAULTS 1
#define MIN_CELL_TEMP_FAULTS 1
//////////////////////////////////////////////////////////////


/* Definitions for stateMachineTask */

// extern osThreadId_t stateMachineTaskHandle;
// extern const osThreadAttr_t stateMachineTask_attributes;

typedef enum {
    Initialize,
    InitializeFault,
    Idle,
    Precharging,
    Run,
    Stop,
    Sleep,
    NormalDangerFault,
    SevereDangerFault,
    NoFault,
    Charging,
    Charged,
    Balancing
} State_t;

typedef State_t (*pfEvent)(void);

typedef struct {
    State_t State;
    pfEvent Event;
} StateMachine;

class StateMachineThread {
    public:
        static void startStateMachine(void *argument);

        static void startMeasurements(void *argument);
        static void stopMeasurements(void *argument);

        static void setState(State_t state);

        static StateMachine* SM;

    private:
        static RTOSThread thread;

        static void runStateMachine(void *arg);

        static void sendCANHeartbeat(void);

        static State_t severeFaultChecking();
        static State_t normalFaultChecking();

        static State_t InitializeEvent(void);
        static State_t InitializeFaultEvent(void);
        static State_t IdleEvent(void);
        static State_t PrechargingEvent(void);
        static State_t RunEvent(void);
        static State_t StopEvent(void);
        static State_t SleepEvent(void);
        static State_t NormalDangerFaultEvent(void);
        static State_t SevereDangerFaultEvent(void);
        static State_t NoFaultEvent(void);
        static State_t ChargingEvent(void);
        static State_t ChargedEvent(void);
        static State_t BalancingEvent(void);
};


