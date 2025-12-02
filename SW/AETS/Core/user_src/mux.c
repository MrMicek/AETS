/*
 * mux.c
 *
 *  Created on: Oct 2, 2025
 *      Author: user
 */


#include "mux.h"
#include "main.h"


void MUX_Init(void)
{
/* Default to internal */
HAL_GPIO_WritePin(GPIO_MUX1_GPIO_Port, GPIO_MUX1_Pin, GPIO_PIN_RESET);
HAL_GPIO_WritePin(GPIO_MUX2_GPIO_Port, GPIO_MUX2_Pin, GPIO_PIN_RESET);
}


void MUX_Set(mux_sel_t sel)
{
/* Simple 2-bit select; adapt truth table to your HW */
if (sel == MUX_INT) {
HAL_GPIO_WritePin(GPIO_MUX1_GPIO_Port, GPIO_MUX1_Pin, GPIO_PIN_SET);
HAL_GPIO_WritePin(GPIO_MUX2_GPIO_Port, GPIO_MUX2_Pin, GPIO_PIN_SET);
} else {
HAL_GPIO_WritePin(GPIO_MUX1_GPIO_Port, GPIO_MUX1_Pin, GPIO_PIN_RESET);
HAL_GPIO_WritePin(GPIO_MUX2_GPIO_Port, GPIO_MUX2_Pin, GPIO_PIN_RESET);
}
}


void MUX_Test(void)
{
HAL_Delay(500);
MUX_Set(MUX_EXT);
HAL_Delay(500);
MUX_Set(MUX_INT);
}
