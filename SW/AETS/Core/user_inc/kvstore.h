#ifndef USER_INC_KVSTORE_H_
#define USER_INC_KVSTORE_H_

#include "main.h"
#include "eeprom.h"
#include "stdint.h"

/* Simple KV store for a single 32-bit counter saved to EEPROM at address 0x0000
 * Layout at addr 0x0000: two alternating slots (seq/counter/crc)
 */

/* If set to 1, the brownout hook will attempt a best-effort immediate blocking EEPROM write
 * from the brownout handler. This increases chances to persist data on fast brownouts
 * but calls blocking HAL I2C functions from the interrupt context (potentially unsafe).
 * Set to 0 to only set a pending flag and let main loop perform the blocking write.
 */
#ifndef KV_IMMEDIATE_ON_BROWNOUT
#define KV_IMMEDIATE_ON_BROWNOUT 1
#endif

void KV_Init(void);
uint32_t KV_GetCounter(void);
void KV_IncCounter(void);
void KV_SaveCounter(void);

/* Call from main context: handle pending brownout save requested from ISR */
void KV_HandlePending(void);

#endif /* USER_INC_KVSTORE_H_ */