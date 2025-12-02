/*
 * ErrReg.h
 *
 *  Created on: 2023
 *      Author: Vojta
 */

#ifndef ERROR_H_
#define ERROR_H_


#include <stdint.h>

/*
 * Error enumeration to be used in all functions returning error states.
 * Values might be added, but never removed. Only positive values shall be specified for each enumerated error.
 */
typedef enum{
	err_Td_Ok = 0,
	err_Td_General = 1,
	err_Td_Param = 2,
	err_Td_Ack = 3,
	err_Td_Nack = 4,
	err_Td_Init = 5,
	err_Td_Busy = 6,
	err_Td_Timeout = 7,
	err_Td_Satur = 8,
	err_Td_Range = 9,
	err_Td_NotExist = 10,
	err_Td_Null = 11,
	err_Td_NotImple = 12,
	err_Td_NotFound = 13,
	err_Td_NotSup = 14,
	err_Td_NotValid = 15,
	err_Td_Overflow = 16,
	err_Td_CRC = 17,
	err_Td_Disabled = 18,
} err_Td;



#define ER_USB_UNDERVOLTAGE				(1 << 0)		//
#define ER_USB_OVERVOLTAGE				(1 << 1)		//
#define ER_DACA_DIAG_FAIL				(1 << 2)		//
#define ER_DACB_DIAG_FAIL				(1 << 3)		//
#define ER_PWM_DIAG_FAIL				(1 << 4)		//
#define ER_CONV_VAL_RANGE_FAIL			(1 << 5)
//TODO definovat dalsi chybove stavy



extern void er_SetErrors(uint32_t errMask);
extern uint32_t er_GetErrors(void);
extern uint8_t er_IsError(uint32_t errMask);
extern void er_Reset(void);

#endif /* ERROR_H_ */
