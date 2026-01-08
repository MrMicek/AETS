#ifndef USER_INC_RELAY_COUNTER_H_
#define USER_INC_RELAY_COUNTER_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32g4xx_hal.h"

void relay_counter_init(void);
void relay_counter_on_relay_change(uint8_t index, bool old_state, bool new_state);
uint64_t relay_counter_get(uint8_t index);
void relay_counter_get_all(uint64_t *out_counts, uint8_t max_len);
void relay_counter_reset(void);
HAL_StatusTypeDef relay_counter_save_now(uint32_t timeout_ms);
HAL_StatusTypeDef relay_counter_load(void);
void relay_counter_periodic_flush(uint32_t now_ms);
HAL_StatusTypeDef relay_counter_emergency_flush(void);

#endif /* USER_INC_RELAY_COUNTER_H_ */
