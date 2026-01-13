/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Green_Pin GPIO_PIN_1
#define LED_Green_GPIO_Port GPIOA
#define TP1_Pin GPIO_PIN_2
#define TP1_GPIO_Port GPIOA
#define LED_Red_Pin GPIO_PIN_3
#define LED_Red_GPIO_Port GPIOA
#define ADD8_Data_Pin GPIO_PIN_4
#define ADD8_Data_GPIO_Port GPIOA
#define ADD4_Pin GPIO_PIN_5
#define ADD4_GPIO_Port GPIOA
#define ADD2_Pin GPIO_PIN_6
#define ADD2_GPIO_Port GPIOA
#define ADD1_Pin GPIO_PIN_7
#define ADD1_GPIO_Port GPIOA
#define TMS_CLK_Pin GPIO_PIN_2
#define TMS_CLK_GPIO_Port GPIOB
#define TMS_CLK_EXTI_IRQn EXTI2_3_IRQn
#define M0_Pin GPIO_PIN_11
#define M0_GPIO_Port GPIOA
#define M0_EXTI_IRQn EXTI4_15_IRQn
#define M1_Pin GPIO_PIN_12
#define M1_GPIO_Port GPIOA
#define M1_EXTI_IRQn EXTI4_15_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
