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
#include "stm32g4xx_hal.h"

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
#define ENCODER_SW_Pin GPIO_PIN_13
#define ENCODER_SW_GPIO_Port GPIOC
#define ENCODER_SW_EXTI_IRQn EXTI15_10_IRQn
#define TRIGGER_OUT_Pin GPIO_PIN_14
#define TRIGGER_OUT_GPIO_Port GPIOC
#define ADC_RELAY4_Pin GPIO_PIN_0
#define ADC_RELAY4_GPIO_Port GPIOC
#define ADC_RELAY3_Pin GPIO_PIN_1
#define ADC_RELAY3_GPIO_Port GPIOC
#define ADC_RELAY2_Pin GPIO_PIN_2
#define ADC_RELAY2_GPIO_Port GPIOC
#define ADC_RELAY1_Pin GPIO_PIN_3
#define ADC_RELAY1_GPIO_Port GPIOC
#define ADC_VBUS_Pin GPIO_PIN_0
#define ADC_VBUS_GPIO_Port GPIOA
#define BUZZER_Pin GPIO_PIN_3
#define BUZZER_GPIO_Port GPIOA
#define OLED_SCK_Pin GPIO_PIN_5
#define OLED_SCK_GPIO_Port GPIOA
#define OLED_CS_Pin GPIO_PIN_6
#define OLED_CS_GPIO_Port GPIOA
#define OLED_MOSI_Pin GPIO_PIN_7
#define OLED_MOSI_GPIO_Port GPIOA
#define OLED_RST_Pin GPIO_PIN_4
#define OLED_RST_GPIO_Port GPIOC
#define GPIO_RELAY1_Pin GPIO_PIN_5
#define GPIO_RELAY1_GPIO_Port GPIOC
#define GPIO_RELAY2_Pin GPIO_PIN_0
#define GPIO_RELAY2_GPIO_Port GPIOB
#define GPIO_RELAY3_Pin GPIO_PIN_1
#define GPIO_RELAY3_GPIO_Port GPIOB
#define GPIO_RELAY4_Pin GPIO_PIN_2
#define GPIO_RELAY4_GPIO_Port GPIOB
#define CAN_STB_Pin GPIO_PIN_11
#define CAN_STB_GPIO_Port GPIOB
#define CAN_RXD_Pin GPIO_PIN_12
#define CAN_RXD_GPIO_Port GPIOB
#define CAN_TXD_Pin GPIO_PIN_13
#define CAN_TXD_GPIO_Port GPIOB
#define GPIO_MOSFET1_Pin GPIO_PIN_14
#define GPIO_MOSFET1_GPIO_Port GPIOB
#define GPIO_MUX1_Pin GPIO_PIN_15
#define GPIO_MUX1_GPIO_Port GPIOB
#define GPIO_MOSFET2_Pin GPIO_PIN_6
#define GPIO_MOSFET2_GPIO_Port GPIOC
#define GPIO_MUX2_Pin GPIO_PIN_7
#define GPIO_MUX2_GPIO_Port GPIOC
#define EEPROM_SCL_Pin GPIO_PIN_8
#define EEPROM_SCL_GPIO_Port GPIOC
#define EEPROM_SDA_Pin GPIO_PIN_9
#define EEPROM_SDA_GPIO_Port GPIOC
#define ENCODER_A_Pin GPIO_PIN_8
#define ENCODER_A_GPIO_Port GPIOA
#define ENCODER_B_Pin GPIO_PIN_9
#define ENCODER_B_GPIO_Port GPIOA
#define USB_DIS_Pin GPIO_PIN_10
#define USB_DIS_GPIO_Port GPIOA
#define PA15_Pin GPIO_PIN_15
#define PA15_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
