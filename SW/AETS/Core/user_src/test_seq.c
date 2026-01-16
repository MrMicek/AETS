/*
 * test_seq.c
 *
 *  Created on: Nov 8, 2025
 */

#include "test_seq.h"
#include "app_params.h"
#include "io_control.h"
#include <string.h>

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

typedef struct {
    relay_params_t relays[4];
    mosfet_params_t mosfets[2];
    trigger_params_t trigger;
} test_seq_params_t;

static test_seq_params_t s_params;
static uint32_t s_relay_remaining[4];
static uint32_t s_mosfet_remaining[2];
static uint8_t s_params_valid = 0;

static uint32_t clamp_delay_ms(int value)
{
    return (value > 0) ? (uint32_t)value : 1U;
}

static uint32_t relay_count_from_k(int value_k)
{
    if (value_k <= 0) {
        return 0;
    }
    return (uint32_t)value_k * 1000U;
}

static void test_seq_reset_remaining(void)
{
    for (uint8_t i = 0; i < 4U; ++i) {
        if (s_params.relays[i].enabled != 0) {
            s_relay_remaining[i] = relay_count_from_k(s_params.relays[i].sw_count_k);
        } else {
            s_relay_remaining[i] = 0;
        }
    }
    for (uint8_t i = 0; i < 2U; ++i) {
        if (s_params.mosfets[i].enabled != 0 && s_params.mosfets[i].ext_control == 0) {
            s_mosfet_remaining[i] = (s_params.mosfets[i].sw_count > 0) ? (uint32_t)s_params.mosfets[i].sw_count : 0U;
        } else {
            s_mosfet_remaining[i] = 0;
        }
    }
}

void test_seq_set_params_current(void)
{
    memcpy(&s_params.relays[0], &g_app_params.relays[0], sizeof(s_params.relays));
    memcpy(&s_params.mosfets[0], &g_app_params.mosfets[0], sizeof(s_params.mosfets));
    memcpy(&s_params.trigger, &g_app_params.trigger, sizeof(s_params.trigger));
    test_seq_reset_remaining();
    s_params_valid = 1;
}

void test_seq_set_params_profile(uint8_t profile_id)
{
    (void)profile_id;
    test_seq_set_params_current();
}

void test_seq_start(uint32_t now_ms)
{
    if (s_params_valid == 0U) {
        test_seq_set_params_current();
    }
    test_seq_reset_remaining();
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
        while (s_relay_idx < 4 && (s_params.relays[s_relay_idx].enabled == 0 || s_relay_remaining[s_relay_idx] == 0U)) {
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
        if (s_relay_remaining[s_relay_idx] > 0U) {
            s_relay_remaining[s_relay_idx]--;
        }
        s_deadline_ms = now_ms + clamp_delay_ms(s_params.relays[s_relay_idx].ton_ms);
        s_state = TEST_SEQ_RELAY_OFF;
        break;
    }
    case TEST_SEQ_RELAY_OFF: {
        io_state_t next = *io_get();
        next.relays[s_relay_idx] = false;
        io_apply(&next);
        s_deadline_ms = now_ms + clamp_delay_ms(s_params.relays[s_relay_idx].toff_ms);
        s_relay_idx++;
        s_state = TEST_SEQ_RELAY_ON;
        break;
    }
    case TEST_SEQ_MOSFET_ON: {
        while (s_mosfet_idx < 2 && (s_params.mosfets[s_mosfet_idx].enabled == 0 ||
                                    s_params.mosfets[s_mosfet_idx].ext_control != 0 ||
                                    s_mosfet_remaining[s_mosfet_idx] == 0U)) {
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
        if (s_mosfet_remaining[s_mosfet_idx] > 0U) {
            s_mosfet_remaining[s_mosfet_idx]--;
        }
        s_deadline_ms = now_ms + clamp_delay_ms(s_params.mosfets[s_mosfet_idx].ton_ms);
        s_state = TEST_SEQ_MOSFET_OFF;
        break;
    }
    case TEST_SEQ_MOSFET_OFF: {
        io_state_t next = *io_get();
        next.mosfet[s_mosfet_idx] = false;
        io_apply(&next);
        s_deadline_ms = now_ms + clamp_delay_ms(s_params.mosfets[s_mosfet_idx].toff_ms);
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

uint32_t test_seq_get_relay_remaining(uint8_t index)
{
    if (index >= 4U) {
        return 0;
    }
    return s_relay_remaining[index];
}

uint32_t test_seq_get_mosfet_remaining(uint8_t index)
{
    if (index >= 2U) {
        return 0;
    }
    return s_mosfet_remaining[index];
}

bool test_seq_relay_is_enabled(uint8_t index)
{
    if (index >= 4U) {
        return false;
    }
    return (s_params.relays[index].enabled != 0);
}

bool test_seq_mosfet_is_enabled(uint8_t index)
{
    if (index >= 2U) {
        return false;
    }
    return (s_params.mosfets[index].enabled != 0) && (s_params.mosfets[index].ext_control == 0);
}
