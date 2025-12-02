/*
 * trigger.h
 *
 *  Created on: Oct 2, 2025
 *      Author: user
 */

#ifndef USER_INC_TRIGGER_H_
#define USER_INC_TRIGGER_H_

#include "main.h"
#include "stdint.h"

void Trigger_Init(void);
void Trigger_Pulse_us(uint32_t width_us);


/* Demo */
void Trigger_TestPulse(void);

#endif /* USER_INC_TRIGGER_H_ */
