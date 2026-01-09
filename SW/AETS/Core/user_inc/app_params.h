/*
 * app_params.h
 *
 *  Created on: Nov 8, 2025
 */

#ifndef USER_INC_APP_PARAMS_H_
#define USER_INC_APP_PARAMS_H_

#include <stdint.h>

typedef struct {
    int enabled;
    int ton_ms;
    int toff_ms;
    int imax_ma;
    int sw_count_k;
} relay_params_t;

typedef struct {
    int enabled;
    int ext_control;
    int ton_ms;
    int toff_ms;
    int sw_count;
} mosfet_params_t;

typedef struct {
    int enable;
    int channel;
} trigger_params_t;

typedef struct {
    int enable;
    int can_enable;
    int usb_enable;
} connectivity_params_t;

typedef struct {
    int buzzer_enable;
    connectivity_params_t connectivity;
    trigger_params_t trigger;
    relay_params_t relays[4];
    mosfet_params_t mosfets[2];
    int relay_health[4];
} app_params_t;

extern app_params_t g_app_params;

void app_params_init(void);

#endif /* USER_INC_APP_PARAMS_H_ */
