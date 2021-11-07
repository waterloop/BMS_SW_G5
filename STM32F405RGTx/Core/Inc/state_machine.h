/*
 * state_machine.h
 *
 *  Created on: Jul. 11, 2021
 *      Author: tiffanywang
 */

#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

#include "wloop_can.h"
#include "cmsis_os.h"

#define TURN_ON_PRECHARGE_PIN() (HAL_GPIO_WritePin(PRECHARGE_GPIO_Port, PRECHARGE_Pin, 1))
#define TURN_OFF_PRECHARGE_PIN() (HAL_GPIO_WritePin(PRECHARGE_GPIO_Port, PRECHARGE_Pin, 0))
#define TURN_ON_CONTACTOR_PIN() (HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 1))
#define TURN_OFF_CONTACTOR_PIN() (HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 0))
#define PRECHARGE_VOLTAGE_THRESHOLD 40.0

#define MAX_VOLTAGE_SEVERE 4.0
#define MIN_VOLTAGE_SEVERE 1.0
#define MAX_TEMP_SEVERE 70.0
#define MAX_CURRENT_SEVERE 50.0

#define MAX_VOLTAGE_NORMAL 3.8
#define MIN_VOLTAGE_NORMAL 1.8
#define MAX_TEMP_NORMAL 60.0
#define MIN_CURRENT_NORMAL 5.0

#define MIN_VOLT_FAULTS 5
#define MIN_TEMP_FAULTS 5

#define ARR_VAL	65535

/* Definitions for stateMachineTask */

// extern osThreadId_t stateMachineTaskHandle;
// extern const osThreadAttr_t stateMachineTask_attributes;

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
