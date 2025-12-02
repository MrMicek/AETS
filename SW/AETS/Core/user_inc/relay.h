/*
 * relay.h
 *
 *  Created on: Oct 2, 2025
 *      Author: user
 */
#include "stdbool.h"
#include "stdint.h"

#ifndef USER_INC_RELAY_H_
#define USER_INC_RELAY_H_

void Relay_Init(void);
void Relay_Set(uint8_t index, bool on);
void Relay_Toggle(uint8_t index);
void Relay_AllOff(void);
void Relay_AllOn(void);


/* Demo */
void Relay_TestToggleAll(uint32_t delay_ms);

#endif /* USER_INC_RELAY_H_ */
