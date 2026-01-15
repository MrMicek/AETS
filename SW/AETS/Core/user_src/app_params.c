/*
 * app_params.c
 *
 *  Created on: Nov 8, 2025
 */

#include "app_params.h"
#include <string.h>

app_params_t g_app_params;

void app_params_init(void)
{
    const app_params_t defaults = {
        .buzzer_enable = 1,
        .connectivity = {
            .enable = 0,
            .can_enable = 0,
            .usb_enable = 0,
        },
        .trigger = {
            .enable = 0,
            .channel = 1,
        },
        .relays = {
            { .enabled = 0, .ton_ms = 0, .toff_ms = 0, .imax_ma = 0, .sw_count_k = 0 },
            { .enabled = 0, .ton_ms = 0, .toff_ms = 0, .imax_ma = 0, .sw_count_k = 0 },
            { .enabled = 0, .ton_ms = 0, .toff_ms = 0, .imax_ma = 0, .sw_count_k = 0 },
            { .enabled = 0, .ton_ms = 0, .toff_ms = 0, .imax_ma = 0, .sw_count_k = 0 },
        },
        .mosfets = {
            { .enabled = 0, .ext_control = 0, .ton_ms = 0, .toff_ms = 0, .sw_count = 0 },
            { .enabled = 0, .ext_control = 0, .ton_ms = 0, .toff_ms = 0, .sw_count = 0 },
        },
        .relay_health = { 100000, 70000, 90000, 80000 },
    };

    memcpy(&g_app_params, &defaults, sizeof(g_app_params));
}
