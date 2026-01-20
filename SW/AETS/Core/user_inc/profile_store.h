/*
 * profile_store.h
 *
 *  Created on: Nov 12, 2025
 */

#ifndef USER_INC_PROFILE_STORE_H_
#define USER_INC_PROFILE_STORE_H_

#include <stdint.h>
#include <stdbool.h>
#include "app_params.h"

#define PROFILE_STORE_COUNT 6U

typedef struct {
    int32_t buzzer_enable;
    connectivity_params_t connectivity;
    trigger_params_t trigger;
    relay_params_t relays[4];
    mosfet_params_t mosfets[2];
} app_profile_t;

bool profile_store_save(uint8_t profile_id, const app_params_t *params);
bool profile_store_load(uint8_t profile_id, app_profile_t *out_profile);
void profile_store_apply(const app_profile_t *profile, app_params_t *params);
void profile_store_from_params(app_profile_t *profile, const app_params_t *params);

#endif /* USER_INC_PROFILE_STORE_H_ */
