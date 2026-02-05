/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, TRIGGER_OUT_Pin|OLED_RST_Pin|GPIO_RELAY1_Pin|GPIO_MOSFET2_Pin
                          |GPIO_MUX2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, OLED_CS_Pin|USB_DIS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_RELAY2_Pin|GPIO_RELAY3_Pin|GPIO_RELAY4_Pin|CAN_STB_Pin
                          |GPIO_MOSFET1_Pin|GPIO_MUX1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : ENCODER_SW_Pin */
  GPIO_InitStruct.Pin = ENCODER_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ENCODER_SW_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TRIGGER_OUT_Pin OLED_RST_Pin GPIO_RELAY1_Pin GPIO_MOSFET2_Pin
                           GPIO_MUX2_Pin */
  GPIO_InitStruct.Pin = TRIGGER_OUT_Pin|OLED_RST_Pin|GPIO_RELAY1_Pin|GPIO_MOSFET2_Pin
                          |GPIO_MUX2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : OLED_CS_Pin USB_DIS_Pin */
  GPIO_InitStruct.Pin = OLED_CS_Pin|USB_DIS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO_RELAY2_Pin GPIO_RELAY3_Pin GPIO_RELAY4_Pin CAN_STB_Pin
                           GPIO_MOSFET1_Pin GPIO_MUX1_Pin */
  GPIO_InitStruct.Pin = GPIO_RELAY2_Pin|GPIO_RELAY3_Pin|GPIO_RELAY4_Pin|CAN_STB_Pin
                          |GPIO_MOSFET1_Pin|GPIO_MUX1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
