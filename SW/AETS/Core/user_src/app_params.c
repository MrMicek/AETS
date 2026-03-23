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
		.current_monitoring_enabled = 0,
        .connectivity = {
            .enable = 0,
            .can_enable = 0,
            .usb_enable = 0,
			.telemetry_period_ms = 1000,
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
        .relay_health_set_k = { 10000, 10000, 10000, 10000 },
        .relay_health_remaining_k = { 10000, 10000, 10000, 10000 },
    };

    memcpy(&g_app_params, &defaults, sizeof(g_app_params));
}
