/*
 * test_seq.c
 *
 *  Created on: Nov 8, 2025
 */

#include "test_seq.h"
#include "app_params.h"
#include "io_control.h"
#include "trigger.h"

typedef enum {
    TEST_SEQ_IDLE = 0,
    TEST_SEQ_RELAY_ON,
    TEST_SEQ_RELAY_OFF,
    TEST_SEQ_MOSFET_ON,
    TEST_SEQ_MOSFET_OFF,
    TEST_SEQ_DONE,
} test_seq_state_t;

static test_seq_state_t s_state = TEST_SEQ_IDLE;
static uint8_t s_relay_idx = 0;
static uint8_t s_mosfet_idx = 0;
static uint32_t s_deadline_ms = 0;

static uint32_t clamp_delay_ms(int value)
{
    return (value > 0) ? (uint32_t)value : 1U;
}

void test_seq_start(uint32_t now_ms)
{
    s_state = TEST_SEQ_RELAY_ON;
    s_relay_idx = 0;
    s_mosfet_idx = 0;
    s_deadline_ms = now_ms;
}

void test_seq_stop(void)
{
    s_state = TEST_SEQ_IDLE;
    io_safe_off();
}

bool test_seq_tick(uint32_t now_ms)
{
    if (s_state == TEST_SEQ_IDLE) {
        return false;
    }

    if (now_ms < s_deadline_ms) {
        return false;
    }

    switch (s_state) {
    case TEST_SEQ_RELAY_ON: {
        while (s_relay_idx < 4 && g_app_params.relays[s_relay_idx].enabled == 0) {
            s_relay_idx++;
        }
        if (s_relay_idx >= 4) {
            s_state = TEST_SEQ_MOSFET_ON;
            s_deadline_ms = now_ms;
            return false;
        }
        io_state_t next = *io_get();
        next.relays[s_relay_idx] = true;
        io_apply(&next);
        if (g_app_params.trigger.enable != 0 && g_app_params.trigger.channel == (int)(s_relay_idx + 1U)) {
            Trigger_Pulse_us(100U);
        }
        s_deadline_ms = now_ms + clamp_delay_ms(g_app_params.relays[s_relay_idx].ton_ms);
        s_state = TEST_SEQ_RELAY_OFF;
        break;
    }
    case TEST_SEQ_RELAY_OFF: {
        io_state_t next = *io_get();
        next.relays[s_relay_idx] = false;
        io_apply(&next);
        s_deadline_ms = now_ms + clamp_delay_ms(g_app_params.relays[s_relay_idx].toff_ms);
        s_relay_idx++;
        s_state = TEST_SEQ_RELAY_ON;
        break;
    }
    case TEST_SEQ_MOSFET_ON: {
        while (s_mosfet_idx < 2) {
            if (g_app_params.mosfets[s_mosfet_idx].enabled != 0 && g_app_params.mosfets[s_mosfet_idx].ext_control == 0) {
                break;
            }
            s_mosfet_idx++;
        }
        if (s_mosfet_idx >= 2) {
            s_state = TEST_SEQ_DONE;
            io_safe_off();
            return true;
        }
        io_state_t next = *io_get();
        next.mux[s_mosfet_idx] = MUX_INT;
        next.mosfet[s_mosfet_idx] = true;
        io_apply(&next);
        s_deadline_ms = now_ms + clamp_delay_ms(g_app_params.mosfets[s_mosfet_idx].ton_ms);
        s_state = TEST_SEQ_MOSFET_OFF;
        break;
    }
    case TEST_SEQ_MOSFET_OFF: {
        io_state_t next = *io_get();
        next.mosfet[s_mosfet_idx] = false;
        io_apply(&next);
        s_deadline_ms = now_ms + clamp_delay_ms(g_app_params.mosfets[s_mosfet_idx].toff_ms);
        s_mosfet_idx++;
        s_state = TEST_SEQ_MOSFET_ON;
        break;
    }
    case TEST_SEQ_DONE:
        return true;

    default:
        break;
    }

    return false;
}
