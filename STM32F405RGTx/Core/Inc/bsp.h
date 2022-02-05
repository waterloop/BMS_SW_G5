// Created by Nushaab Syed

// Contactor macros
#define TURN_ON_CONTACTOR_PIN() (HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 1))
#define TURN_OFF_CONTACTOR_PIN() (HAL_GPIO_WritePin(CONTACTOR_GPIO_Port, CONTACTOR_Pin, 0))

// Precharger macros
#define TURN_ON_PRECHARGE_PIN() (HAL_GPIO_WritePin(PRECHARGE_GPIO_Port, PRECHARGE_Pin, 1))
#define TURN_OFF_PRECHARGE_PIN() (HAL_GPIO_WritePin(PRECHARGE_GPIO_Port, PRECHARGE_Pin, 0))

// Slave device macros


// CAN fault macros


// UART fault macros


// State machine hard fault transition macros
