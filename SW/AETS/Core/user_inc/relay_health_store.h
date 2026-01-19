/*
 * relay_health_store.h
 *
 *  Created on: Nov 12, 2025
 */

#ifndef USER_INC_RELAY_HEALTH_STORE_H_
#define USER_INC_RELAY_HEALTH_STORE_H_

#include <stdint.h>
#include "stm32g4xx_hal.h"

void relay_health_init(void);
HAL_StatusTypeDef relay_health_save_now(uint32_t timeout_ms);
void relay_health_request_pending(void);
void relay_health_handle_pending(void);
void relay_health_update_from_test(void);

#endif /* USER_INC_RELAY_HEALTH_STORE_H_ */
