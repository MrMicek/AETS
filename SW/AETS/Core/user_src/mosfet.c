/*
 * mosfet.c
 *
 *  Created on: Oct 2, 2025
 *      Author: user
 */


#include "mosfet.h"
#include "main.h"
#include "app_params.h"

static GPIO_TypeDef* const mosfet_ports[2] = { GPIO_MOSFET1_GPIO_Port, GPIO_MOSFET2_GPIO_Port };
static const uint16_t mosfet_pins [2] = { GPIO_MOSFET1_Pin, GPIO_MOSFET2_Pin };


void MOSFET_Init(void)
{
/* GPIOs configured by MX_GPIO_Init() */
for (int i=0;i<2;++i) HAL_GPIO_WritePin(mosfet_ports[i], mosfet_pins[i], GPIO_PIN_RESET);
}


void MOSFET_Set(mosfet_id_t id, bool on)
{
	if (id <= MOSFET2) {
		bool allow = (g_app_params.mosfets[id].enabled != 0) && (g_app_params.mosfets[id].ext_control == 0);
		HAL_GPIO_WritePin(mosfet_ports[id], mosfet_pins[id], (on && allow) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}


void MOSFET_TestToggle(uint32_t delay_ms)
{
MOSFET_Set(MOSFET1, true); HAL_Delay(delay_ms);
MOSFET_Set(MOSFET1, false); HAL_Delay(delay_ms);
MOSFET_Set(MOSFET2, true); HAL_Delay(delay_ms);
MOSFET_Set(MOSFET2, false); HAL_Delay(delay_ms);
}
