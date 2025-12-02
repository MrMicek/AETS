/*
 * encoder.h
 *
 *  Created on: Oct 13, 2025
 *      Author: uiv10467
 */

#ifndef USER_INC_ENCODER_H_
#define USER_INC_ENCODER_H_


#include "comuser.h"
#include "main.h"
#include "tim.h"
#include "stm32g4xx_hal.h"


typedef enum {
	NONE = 0, DOWN = 1, UP = 2
} EncoderDirection_Td;

void Encoder_init_once(void);
EncoderDirection_Td Encoder_read();


#endif /* USER_INC_ENCODER_H_ */
