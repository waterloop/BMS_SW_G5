/*
 * state_machine.h
 *
 *  Created on: Jul. 11, 2021
 *      Author: tiffanywang
 */

#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

#include "cmsis_os.h"


/* Definitions for stateMachineTask */
osThreadId_t stateMachineTaskHandle;
const osThreadAttr_t stateMachineTask_attributes;

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

/**
  * @brief  Function implementing the StartStateMachine thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartStateMachine */
void StartStateMachine(void *argument);

void StartMeasurements(void *argument);



#endif /* _STATE_MACHINE_H_ */
