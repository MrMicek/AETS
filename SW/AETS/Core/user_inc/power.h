/*
 * power.h
 *
 *  Created on: Oct 2, 2025
 *      Author: user
 */

#ifndef USER_INC_POWER_H_
#define USER_INC_POWER_H_

#include "main.h"
#include "error.h"
#include "stm32g4xx_hal.h"
#include "stdint.h"



__attribute__((weak)) void Power_OnBrownout(void);

err_Td Power_InitBrownout(void);
/* Demo */
void Power_TestBrownout(void);

#endif /* USER_INC_POWER_H_ */
