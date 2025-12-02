/*
 * trigger.c
 *
 *  Created on: Oct 2, 2025
 *      Author: user
 */


#include "trigger.h"


/* DWT delay for ~us accuracy */
static inline void DWT_Init(void) {
CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
DWT->CYCCNT = 0;
DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}
static inline void delay_us(uint32_t us) {
uint32_t cycles = (HAL_RCC_GetHCLKFreq()/1000000) * us;
uint32_t start = DWT->CYCCNT;
while ((DWT->CYCCNT - start) < cycles) {}
}


void Trigger_Init(void)
{
DWT_Init();
HAL_GPIO_WritePin(TRIGGER_OUT_GPIO_Port, TRIGGER_OUT_Pin, GPIO_PIN_RESET);
}


void Trigger_Pulse_us(uint32_t width_us)
{
HAL_GPIO_WritePin(TRIGGER_OUT_GPIO_Port, TRIGGER_OUT_Pin, GPIO_PIN_SET);
delay_us(width_us);
HAL_GPIO_WritePin(TRIGGER_OUT_GPIO_Port, TRIGGER_OUT_Pin, GPIO_PIN_RESET);
}


void Trigger_TestPulse(void)
{
while (1) {
		for (int i = 1; i < 101; i++) {
			Trigger_Pulse_us(10 * i);
			HAL_Delay(1500);
		}
}
}
