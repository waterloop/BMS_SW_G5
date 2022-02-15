// Created by Nushaab Syed

#pragma once

// Contactor macros
#define TURN_ON_CONTACTOR_PIN() (HAL_GPIO_WritePin(CONT1_GPIO_Port, CONT1_Pin, 1))
#define TURN_OFF_CONTACTOR_PIN() (HAL_GPIO_WritePin(CONT1_GPIO_Port, CONT1_Pin, 0))

// Precharger macros
#define TURN_ON_PRECHARGE_PIN() (HAL_GPIO_WritePin(PRECHARGE_GPIO_Port, PRECHARGE_Pin, 1))
#define TURN_OFF_PRECHARGE_PIN() (HAL_GPIO_WritePin(PRECHARGE_GPIO_Port, PRECHARGE_Pin, 0))

