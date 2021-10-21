#pragma once

/*
 * state_machine.h
 *
 *      Created on: Jul. 11, 2021
 *      Author: tiffanywang
 */


#include "cmsis_os.h"

typedef enum {
    Initialize,
    Idle,
    Precharging,
    Run,
    Stop,
    Sleep,
    NormalDangerFault,
    SevereDangerFault,
    Charging,
    Charged,
    Balancing
} State_t;

typedef State_t (*pfEvent)(void);

typedef struct {
    State_t State;
    pfEvent Event;
} StateMachine;

State_t InitializeEvent(void);
State_t IdleEvent(void);
State_t PrechargingEvent(void);
State_t RunEvent(void);
State_t StopEvent(void);
State_t SleepEvent(void);
State_t NormalDangerFaultEvent(void);
State_t SevereDangerFaultEvent(void);
State_t ChargingEvent(void);
State_t ChargedEvent(void);
State_t BalancingEvent(void);

