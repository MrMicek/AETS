/*
 * eeprom.h
 *
 *  Created on: Oct 2, 2025
 *      Author: user
 */

#ifndef USER_INC_EEPROM_H_
#define USER_INC_EEPROM_H_

#include "main.h"
#include "stdint.h"
#include "string.h"

#define EEPROM_I2C_ADDR (0x50 << 1) /* 7-bit addr 0x50 */
#define EEPROM_PAGE_SIZE 128
#define EEPROM_MEM_ADDR_BYTES 2


void EEPROM_Init(void);
HAL_StatusTypeDef EEPROM_Write(uint16_t mem_addr, const uint8_t* data, uint16_t len);
HAL_StatusTypeDef EEPROM_Read (uint16_t mem_addr, uint8_t* data, uint16_t len);
HAL_StatusTypeDef EEPROM_WriteTimeout(uint16_t mem_addr, const uint8_t* data, uint16_t len, uint32_t timeout_ms);


#endif /* USER_INC_EEPROM_H_ */
