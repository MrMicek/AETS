/*
 * telemetry.c
 *
 *  Created on: Nov 14, 2025
 */

#include "telemetry.h"
#include "app_params.h"
#include "app_sm.h"
#include "canfd.h"
#include "comuser.h"
#include "io_control.h"
#include "test_seq.h"
#include "stdio.h"

#define TELEMETRY_USB_PERIOD_MS 2000U

static uint32_t s_last_usb_ms = 0U;

static void fill_telemetry(cfd_telemetry_t *out)
{
    if (out == NULL) {
        return;
    }
    const io_state_t *io = io_get();
    for (uint8_t i = 0; i < 4U; ++i) {
        out->relay_remaining[i] = test_seq_get_relay_remaining(i);
        out->relay_state[i] = io->relays[i] ? 1U : 0U;
        out->relay_current_ma[i] = app_get_relay_current_ma(i);
    }
    for (uint8_t i = 0; i < 2U; ++i) {
        out->mosfet_state[i] = io->mosfet[i] ? 1U : 0U;
        out->mux_state[i] = (io->mux[i] == MUX_EXT) ? 1U : 0U;
    }
}

void telemetry_tick(uint32_t now_ms)
{
    cfd_telemetry_t telemetry;
    telemetry.timestamp_ms = now_ms;
    fill_telemetry(&telemetry);

    if (g_app_params.connectivity.can_enable != 0) {
        cfd_HandleCommunication(&telemetry, now_ms);
    }

    if (g_app_params.connectivity.usb_enable != 0) {
        if ((now_ms - s_last_usb_ms) >= TELEMETRY_USB_PERIOD_MS) {
            s_last_usb_ms = now_ms;
            comu_SendF("out %lu ", (unsigned long)now_ms);
            for (uint8_t i = 0; i < 4U; ++i) {
                comu_SendF("r%u %lu %u %lu ", (unsigned)(i + 1U),
                           (unsigned long)telemetry.relay_remaining[i],
                           (unsigned)telemetry.relay_state[i],
                           (unsigned long)telemetry.relay_current_ma[i]);
            }
            comu_SendF("m1 %u m2 %u\r\n",
                       (unsigned)telemetry.mosfet_state[0],
                       (unsigned)telemetry.mosfet_state[1]);
        }
    }
}
