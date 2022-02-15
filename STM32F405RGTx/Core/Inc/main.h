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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC1_CURRENT_Pin GPIO_PIN_0
#define ADC1_CURRENT_GPIO_Port GPIOC
#define ADC1_VBATT_Pin GPIO_PIN_1
#define ADC1_VBATT_GPIO_Port GPIOC
#define ADC1_VCAP_Pin GPIO_PIN_2
#define ADC1_VCAP_GPIO_Port GPIOC
#define ADC1_CONTACTOR_Pin GPIO_PIN_3
#define ADC1_CONTACTOR_GPIO_Port GPIOC
#define ADC1_THERMISTOR_Pin GPIO_PIN_1
#define ADC1_THERMISTOR_GPIO_Port GPIOA
#define ADC1_IN2_Pin GPIO_PIN_2
#define ADC1_IN2_GPIO_Port GPIOA
#define CONT2_Pin GPIO_PIN_3
#define CONT2_GPIO_Port GPIOA
#define CS1_Pin GPIO_PIN_4
#define CS1_GPIO_Port GPIOA
#define CS2_Pin GPIO_PIN_4
#define CS2_GPIO_Port GPIOC
#define CONT1_Pin GPIO_PIN_1
#define CONT1_GPIO_Port GPIOB
#define PRECHARGE_Pin GPIO_PIN_2
#define PRECHARGE_GPIO_Port GPIOB
#define EXT_LED_Pin GPIO_PIN_10
#define EXT_LED_GPIO_Port GPIOB
#define BUTTON_Pin GPIO_PIN_11
#define BUTTON_GPIO_Port GPIOB
#define G_Pin GPIO_PIN_8
#define G_GPIO_Port GPIOA
#define B_Pin GPIO_PIN_9
#define B_GPIO_Port GPIOA
#define R_Pin GPIO_PIN_10
#define R_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define TIMING_Pin GPIO_PIN_15
#define TIMING_GPIO_Port GPIOA
#define DEBUG_Pin GPIO_PIN_3
#define DEBUG_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
