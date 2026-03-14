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
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, TRIGGER_OUT_Pin|NOTUSED_Pin|OLED_RST_Pin|GPIO_RELAY1_Pin
                          |GPIO_MOSFET2_Pin|GPIO_MUX2_Pin|NOTUSEDC10_Pin|NOTUSEDC11_Pin
                          |NOTUSEDC12_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, NOTUSEDA1_Pin|NOTUSEDA2_Pin|NOTUSEDA4_Pin|OLED_CS_Pin
                          |USB_DIS_Pin|NOTUSEDA15_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_RELAY2_Pin|GPIO_RELAY3_Pin|GPIO_RELAY4_Pin|NOTUSEDB10_Pin
                          |CAN_STB_Pin|GPIO_MOSFET1_Pin|GPIO_MUX1_Pin|NOTUSEDB3_Pin
                          |NOTUSEDB4_Pin|NOTUSEDB5_Pin|NOTUSEDB6_Pin|NOTUSEDB7_Pin
                          |NOTUSEDB9_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(NOTUSEDD2_GPIO_Port, NOTUSEDD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : ENCODER_SW_Pin */
  GPIO_InitStruct.Pin = ENCODER_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ENCODER_SW_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TRIGGER_OUT_Pin NOTUSED_Pin OLED_RST_Pin GPIO_RELAY1_Pin
                           GPIO_MOSFET2_Pin GPIO_MUX2_Pin NOTUSEDC10_Pin NOTUSEDC11_Pin
                           NOTUSEDC12_Pin */
  GPIO_InitStruct.Pin = TRIGGER_OUT_Pin|NOTUSED_Pin|OLED_RST_Pin|GPIO_RELAY1_Pin
                          |GPIO_MOSFET2_Pin|GPIO_MUX2_Pin|NOTUSEDC10_Pin|NOTUSEDC11_Pin
                          |NOTUSEDC12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : NOTUSEDA1_Pin NOTUSEDA2_Pin NOTUSEDA4_Pin OLED_CS_Pin
                           USB_DIS_Pin NOTUSEDA15_Pin */
  GPIO_InitStruct.Pin = NOTUSEDA1_Pin|NOTUSEDA2_Pin|NOTUSEDA4_Pin|OLED_CS_Pin
                          |USB_DIS_Pin|NOTUSEDA15_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO_RELAY2_Pin GPIO_RELAY3_Pin GPIO_RELAY4_Pin NOTUSEDB10_Pin
                           CAN_STB_Pin GPIO_MOSFET1_Pin GPIO_MUX1_Pin NOTUSEDB3_Pin
                           NOTUSEDB4_Pin NOTUSEDB5_Pin NOTUSEDB6_Pin NOTUSEDB7_Pin
                           NOTUSEDB9_Pin */
  GPIO_InitStruct.Pin = GPIO_RELAY2_Pin|GPIO_RELAY3_Pin|GPIO_RELAY4_Pin|NOTUSEDB10_Pin
                          |CAN_STB_Pin|GPIO_MOSFET1_Pin|GPIO_MUX1_Pin|NOTUSEDB3_Pin
                          |NOTUSEDB4_Pin|NOTUSEDB5_Pin|NOTUSEDB6_Pin|NOTUSEDB7_Pin
                          |NOTUSEDB9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : NOTUSEDD2_Pin */
  GPIO_InitStruct.Pin = NOTUSEDD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(NOTUSEDD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
