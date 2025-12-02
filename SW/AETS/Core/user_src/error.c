/*
 * ErrReg.c
 *
 *  Created on: 2023
 *      Author: Vojta
 */


#include "error.h"
#include "utility.h"
#include "stm32g4xx_hal.h"

static uint32_t ErrorRegister = 0x00;


/*
 * Set bits defined with errMask parameter (see header file).
 * Multiple errors might be set at the same time using bitwise addition (err1 | err2)
 */
void er_SetErrors(uint32_t errMask){
	ErrorRegister |= errMask;
}


/*
 * Returns full content of error register.
 */
uint32_t er_GetErrors(void){
	return ErrorRegister;
}


/*
 * Checks whether specified error is set in error register.
 */
uint8_t er_IsError(uint32_t errMask){
	return ErrorRegister & errMask;
}


/*
 * Resets error register to zero.
 */
void er_Reset(void){
	ErrorRegister = 0;
}


/*
 * Save error register into permament storage (flash memory)
 */
void er_SaveToFlashMem(void){
	//NOT YET IMPLEMENTED
	//Check supply voltage and MCU temperature
	//If voltage is high enough and temperature low enough write the content of error register into flash
	//Do not ever write into flash above 85�C and when power supply for whole write cycle is not guaranteed
	//In general this functionality is unsafe and could lead to field issues in the future
}
