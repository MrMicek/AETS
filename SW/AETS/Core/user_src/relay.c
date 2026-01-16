/*
 * relay.c
 *
 *  Created on: Oct 2, 2025
 *      Author: user
 */


#include "relay.h"
#include "main.h"
#include "app_params.h"

static GPIO_TypeDef* const relay_ports[4] = { GPIO_RELAY1_GPIO_Port, GPIO_RELAY2_GPIO_Port, GPIO_RELAY3_GPIO_Port, GPIO_RELAY4_GPIO_Port };
static const uint16_t relay_pins [4] = { GPIO_RELAY1_Pin, GPIO_RELAY2_Pin, GPIO_RELAY3_Pin, GPIO_RELAY4_Pin };


void Relay_Init(void)
{
/* GPIOs are configured by MX_GPIO_Init(); nothing to do here. */
Relay_AllOff();
}


void Relay_Set(uint8_t index, bool on)
{
	if (index < 4) {
		bool allow = (g_app_params.relays[index].enabled != 0);
		HAL_GPIO_WritePin(relay_ports[index], relay_pins[index], (on && allow) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}


void Relay_Toggle(uint8_t index)
{
if (index < 4) {
HAL_GPIO_TogglePin(relay_ports[index], relay_pins[index]);
}
}


void Relay_AllOff(void)
{
for (uint8_t i=0; i<4; ++i) Relay_Set(i, false);
}


void Relay_AllOn(void)
{
for (uint8_t i=0; i<4; ++i) Relay_Set(i, true);
}


void Relay_TestToggleAll(uint32_t delay_ms)
{
for (uint8_t i=0; i<4; ++i) { Relay_Toggle(i); HAL_Delay(delay_ms); }
}
