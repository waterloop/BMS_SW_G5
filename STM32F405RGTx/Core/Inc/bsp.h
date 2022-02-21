// Created by Nushaab Syed

#pragma once

// Contactor macros
#define TURN_ON_CONT1_PIN() (HAL_GPIO_WritePin(CONT1_GPIO_Port, CONT1_Pin, (GPIO_PinState)1))
#define TURN_OFF_CONT1_PIN() (HAL_GPIO_WritePin(CONT1_GPIO_Port, CONT1_Pin, (GPIO_PinState)0))

#define TURN_ON_CONT2_PIN() (HAL_GPIO_WritePin(CONT2_GPIO_Port, CONT2_Pin, (GPIO_PinState)1))
#define TURN_OFF_CONT2_PIN() (HAL_GPIO_WritePin(CONT2_GPIO_Port, CONT2_Pin, (GPIO_PinState)0))

// Precharger macros
#define TURN_ON_PRECHARGE_PIN() (HAL_GPIO_WritePin(PRECHARGE_GPIO_Port, PRECHARGE_Pin, (GPIO_PinState)1))
#define TURN_OFF_PRECHARGE_PIN() (HAL_GPIO_WritePin(PRECHARGE_GPIO_Port, PRECHARGE_Pin, (GPIO_PinState)0))

// Debug pin macros
#define TURN_ON_DEBUG_PIN() (DEBUG_GPIO_Port |= Debug_Pin)
#define TURN_OFF_DEBUG_PIN() (DEBUG_GPIO_Port &= ~(Debug_Ping))

// Button pin macros
#define GET_BUTTON_PIN() (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin))