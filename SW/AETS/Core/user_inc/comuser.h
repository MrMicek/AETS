/*
 * comuser.h
 *
 *  Created on: 2023
 *      Author: Standa
 */

#ifndef COMUSER_H_
#define COMUSER_H_


#include "error.h"
#include <stdint.h>



extern void comu_Init(void);
extern err_Td comu_SendF(char *format, ...);
extern err_Td comu_Receive(uint8_t* buf, uint32_t len);

extern void comu_HandleCommunication(void);


#endif /* COMUSER_H_ */
