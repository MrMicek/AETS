/*
 * mosfet.h
 *
 *  Created on: Oct 2, 2025
 *      Author: user
 */

#ifndef USER_INC_MOSFET_H_
#define USER_INC_MOSFET_H_

#include "stdbool.h"
#include "stdint.h"

typedef enum { MOSFET1=0, MOSFET2=1 } mosfet_id_t;


void MOSFET_Init(void);
void MOSFET_Set(mosfet_id_t id, bool on);


/* Demo */
void MOSFET_TestToggle(uint32_t delay_ms);

#endif /* USER_INC_MOSFET_H_ */
