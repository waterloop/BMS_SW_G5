/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ltc6813.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern ADC_HandleTypeDef hadc1;
extern CAN_HandleTypeDef hcan1;
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart1;

extern Ltc6813 ltc6813;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CS_Pin GPIO_PIN_4
#define CS_GPIO_Port GPIOA
#define CS2_Pin GPIO_PIN_4
#define CS2_GPIO_Port GPIOC
#define CONTACTOR_Pin GPIO_PIN_1
#define CONTACTOR_GPIO_Port GPIOB
#define PRECHARGE_Pin GPIO_PIN_2
#define PRECHARGE_GPIO_Port GPIOB
#define EXT_LED_Pin GPIO_PIN_10
#define EXT_LED_GPIO_Port GPIOB
#define TIM3_CH1_IMD_IN_Pin GPIO_PIN_6
#define TIM3_CH1_IMD_IN_GPIO_Port GPIOC
#define TIM1_CH1_BLUE_Pin GPIO_PIN_8
#define TIM1_CH1_BLUE_GPIO_Port GPIOA
#define TIM1_CH2_GREEN_Pin GPIO_PIN_9
#define TIM1_CH2_GREEN_GPIO_Port GPIOA
#define TIM1_CH3_RED_Pin GPIO_PIN_10
#define TIM1_CH3_RED_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
