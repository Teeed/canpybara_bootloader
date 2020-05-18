/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define ADDR0_Pin GPIO_PIN_0
#define ADDR0_GPIO_Port GPIOA
#define ADDR1_Pin GPIO_PIN_1
#define ADDR1_GPIO_Port GPIOA
#define ADDR2_Pin GPIO_PIN_2
#define ADDR2_GPIO_Port GPIOA
#define ADDR3_Pin GPIO_PIN_3
#define ADDR3_GPIO_Port GPIOA
#define ADDR4_Pin GPIO_PIN_4
#define ADDR4_GPIO_Port GPIOA
#define ADDR5_Pin GPIO_PIN_5
#define ADDR5_GPIO_Port GPIOA
#define ADDR6_Pin GPIO_PIN_6
#define ADDR6_GPIO_Port GPIOA
#define IN0_Pin GPIO_PIN_0
#define IN0_GPIO_Port GPIOB
#define IN0_EXTI_IRQn EXTI0_IRQn
#define IN1_Pin GPIO_PIN_1
#define IN1_GPIO_Port GPIOB
#define IN1_EXTI_IRQn EXTI1_IRQn
#define IN2_Pin GPIO_PIN_2
#define IN2_GPIO_Port GPIOB
#define OUT2_Pin GPIO_PIN_10
#define OUT2_GPIO_Port GPIOB
#define OUT3_Pin GPIO_PIN_11
#define OUT3_GPIO_Port GPIOB
#define OUT4_Pin GPIO_PIN_12
#define OUT4_GPIO_Port GPIOB
#define OUT5_Pin GPIO_PIN_13
#define OUT5_GPIO_Port GPIOB
#define RELAY0_Pin GPIO_PIN_14
#define RELAY0_GPIO_Port GPIOB
#define RELAY1_Pin GPIO_PIN_15
#define RELAY1_GPIO_Port GPIOB
#define IN3_Pin GPIO_PIN_3
#define IN3_GPIO_Port GPIOB
#define IN4_Pin GPIO_PIN_4
#define IN4_GPIO_Port GPIOB
#define IN5_Pin GPIO_PIN_5
#define IN5_GPIO_Port GPIOB
#define OUT0_Pin GPIO_PIN_6
#define OUT0_GPIO_Port GPIOB
#define OUT1_Pin GPIO_PIN_7
#define OUT1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
