/*
 * eeprom.c
 *
 *  Created on: Oct 2, 2025
 *      Author: user
 */


#include "eeprom.h"
extern I2C_HandleTypeDef hi2c3;
#include "comuser.h"
#include "i2c.h"
#include "main.h"


static HAL_StatusTypeDef eeprom_wait_ready(uint32_t timeout_ms)
{
uint32_t start = HAL_GetTick();
while (HAL_GetTick() - start < timeout_ms) {
if (HAL_I2C_IsDeviceReady(&hi2c3, EEPROM_I2C_ADDR, 1, 1) == HAL_OK)
return HAL_OK;
}
return HAL_TIMEOUT;
}


void EEPROM_Init(void)
{
/* Nothing specific */
}


HAL_StatusTypeDef EEPROM_Write(uint16_t mem_addr, const uint8_t* data, uint16_t len)
{
HAL_StatusTypeDef st = HAL_OK;
uint16_t offset = 0;
while (offset < len) {
uint16_t page_off = mem_addr % EEPROM_PAGE_SIZE;
uint16_t chunk = EEPROM_PAGE_SIZE - page_off;
if (chunk > (len - offset)) chunk = len - offset;


uint8_t buf[EEPROM_PAGE_SIZE + EEPROM_MEM_ADDR_BYTES];
buf[0] = (uint8_t)(mem_addr >> 8);
buf[1] = (uint8_t)(mem_addr & 0xFF);
memcpy(&buf[2], &data[offset], chunk);


st = HAL_I2C_Master_Transmit(&hi2c3, EEPROM_I2C_ADDR, buf, chunk + 2, HAL_MAX_DELAY);
if (st != HAL_OK) {
    int i2cerr = (int)HAL_I2C_GetError(&hi2c3);
    comu_SendF("EEPROM_Write: HAL_I2C_Master_Transmit FAIL st=%d i2c_err=0x%04x addr=0x%04x len=%u\r\n", (int)st, i2cerr, (unsigned int)mem_addr, (unsigned int)chunk);
    return st;
}
eeprom_wait_ready(10);
mem_addr += chunk;
offset += chunk;
}
return st;
}


HAL_StatusTypeDef EEPROM_Read(uint16_t mem_addr, uint8_t* data, uint16_t len)
{
uint8_t addr[2] = { (uint8_t)(mem_addr >> 8), (uint8_t)mem_addr };
HAL_StatusTypeDef st = HAL_I2C_Master_Transmit(&hi2c3, EEPROM_I2C_ADDR, addr, 2, HAL_MAX_DELAY);
if (st != HAL_OK) {
    int i2cerr = (int)HAL_I2C_GetError(&hi2c3);
    comu_SendF("EEPROM_Read: addr transmit FAIL st=%d i2c_err=0x%04x mem_addr=0x%04x\r\n", (int)st, i2cerr, (unsigned int)mem_addr);
    return st;
}
st = HAL_I2C_Master_Receive(&hi2c3, EEPROM_I2C_ADDR, data, len, HAL_MAX_DELAY);
if (st != HAL_OK) {
    int i2cerr = (int)HAL_I2C_GetError(&hi2c3);
    comu_SendF("EEPROM_Read: master receive FAIL st=%d i2c_err=0x%04x mem_addr=0x%04x len=%u\r\n", (int)st, i2cerr, (unsigned int)mem_addr, (unsigned int)len);
}
return st;
}


/* New: write with a limited timeout (best-effort). Useful from emergency contexts. */
HAL_StatusTypeDef EEPROM_WriteTimeout(uint16_t mem_addr, const uint8_t* data, uint16_t len, uint32_t timeout_ms)
{
HAL_StatusTypeDef st = HAL_OK;
uint16_t offset = 0;

while (offset < len) {
uint16_t page_off = mem_addr % EEPROM_PAGE_SIZE;
uint16_t chunk = EEPROM_PAGE_SIZE - page_off;
if (chunk > (len - offset)) chunk = len - offset;

uint8_t buf[EEPROM_PAGE_SIZE + EEPROM_MEM_ADDR_BYTES];
buf[0] = (uint8_t)(mem_addr >> 8);
buf[1] = (uint8_t)(mem_addr & 0xFF);
memcpy(&buf[2], &data[offset], chunk);

// Use provided timeout for the transmit and waiting for device ready
st = HAL_I2C_Master_Transmit(&hi2c3, EEPROM_I2C_ADDR, buf, chunk + 2, timeout_ms);
if (st != HAL_OK) {
    int i2cerr = (int)HAL_I2C_GetError(&hi2c3);
    comu_SendF("EEPROM_WriteTimeout: transmit FAIL st=%d i2c_err=0x%04x addr=0x%04x len=%u timeout=%lums\r\n", (int)st, i2cerr, (unsigned int)mem_addr, (unsigned int)chunk, (unsigned long)timeout_ms);
    return st;
}
if (eeprom_wait_ready(timeout_ms) != HAL_OK) {
    comu_SendF("EEPROM_WriteTimeout: wait READY TIMEOUT addr=0x%04x timeout=%lums\r\n", (unsigned int)mem_addr, (unsigned long)timeout_ms);
    return HAL_TIMEOUT;
}
mem_addr += chunk;
offset += chunk;
}
return st;
}


