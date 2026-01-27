/*
 * test_seq.c
 *
 *  Created on: Nov 8, 2025
 */

#include "test_seq.h"
#include "app_params.h"
#include "io_control.h"
#include "profile_store.h"
#include <string.h>

typedef enum {
    TEST_SEQ_IDLE = 0,
    TEST_SEQ_RUNNING,
    TEST_SEQ_DONE
} test_seq_state_t;

typedef enum {
    TEST_SEQ_PHASE_ON = 0,
    TEST_SEQ_PHASE_OFF,
    TEST_SEQ_PHASE_DONE
} test_seq_phase_t;

static test_seq_state_t s_state = TEST_SEQ_IDLE;
static test_seq_phase_t s_relay_phase = TEST_SEQ_PHASE_ON;
static test_seq_phase_t s_mosfet_phase = TEST_SEQ_PHASE_ON;
static uint8_t s_relay_idx = 0;
static uint8_t s_mosfet_idx = 0;
static uint32_t s_relay_deadline_ms = 0;
static uint32_t s_mosfet_deadline_ms = 0;

typedef struct {
    relay_params_t relays[4];
    mosfet_params_t mosfets[2];
    trigger_params_t trigger;
} test_seq_params_t;

static test_seq_params_t s_params;
static uint32_t s_relay_remaining[4];
static uint32_t s_relay_initial[4];
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
        s_relay_initial[i] = s_relay_remaining[i];
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
    app_profile_t profile;
    if (!profile_store_load(profile_id, &profile)) {
        test_seq_set_params_current();
        return;
    }
    memcpy(&s_params.relays[0], &profile.relays[0], sizeof(s_params.relays));
    memcpy(&s_params.mosfets[0], &profile.mosfets[0], sizeof(s_params.mosfets));
    memcpy(&s_params.trigger, &profile.trigger, sizeof(s_params.trigger));
    test_seq_reset_remaining();
    s_params_valid = 1;
}

void test_seq_start(uint32_t now_ms)
{
    if (s_params_valid == 0U) {
        test_seq_set_params_current();
    }
    test_seq_reset_remaining();
    s_state = TEST_SEQ_RUNNING;
    s_relay_phase = TEST_SEQ_PHASE_ON;
    s_mosfet_phase = TEST_SEQ_PHASE_ON;
    s_relay_idx = 0;
    s_mosfet_idx = 0;
    s_relay_deadline_ms = now_ms;
    s_mosfet_deadline_ms = now_ms;
}

void test_seq_stop(void)
{
    s_state = TEST_SEQ_IDLE;
    s_relay_phase = TEST_SEQ_PHASE_DONE;
    s_mosfet_phase = TEST_SEQ_PHASE_DONE;
    io_safe_off();
}

static void test_seq_step_relays(uint32_t now_ms)
{
    if (s_relay_phase == TEST_SEQ_PHASE_DONE) {
        return;
    }

    if (now_ms < s_relay_deadline_ms) {
        return;
    }

    if (s_relay_phase == TEST_SEQ_PHASE_ON) {
        while (s_relay_idx < 4U &&
               (s_params.relays[s_relay_idx].enabled == 0 || s_relay_remaining[s_relay_idx] == 0U)) {
            s_relay_idx++;
        }
        if (s_relay_idx >= 4U) {
            s_relay_phase = TEST_SEQ_PHASE_DONE;
            return;
        }
        io_state_t next = *io_get();
        next.relays[s_relay_idx] = true;
        io_apply(&next);
        if (s_relay_remaining[s_relay_idx] > 0U) {
            s_relay_remaining[s_relay_idx]--;
        }
        s_relay_deadline_ms = now_ms + clamp_delay_ms(s_params.relays[s_relay_idx].ton_ms);
        s_relay_phase = TEST_SEQ_PHASE_OFF;
        return;
    }

    if (s_relay_phase == TEST_SEQ_PHASE_OFF) {
        io_state_t next = *io_get();
        next.relays[s_relay_idx] = false;
        io_apply(&next);
        s_relay_deadline_ms = now_ms + clamp_delay_ms(s_params.relays[s_relay_idx].toff_ms);
        if (s_relay_remaining[s_relay_idx] == 0U) {
            s_relay_idx++;
        }
        s_relay_phase = TEST_SEQ_PHASE_ON;
        return;
    }
}

static void test_seq_step_mosfets(uint32_t now_ms)
{
    if (s_mosfet_phase == TEST_SEQ_PHASE_DONE) {
        return;
    }

    if (now_ms < s_mosfet_deadline_ms) {
        return;
    }

    if (s_mosfet_phase == TEST_SEQ_PHASE_ON) {
        while (s_mosfet_idx < 2U && (s_params.mosfets[s_mosfet_idx].enabled == 0 ||
                                     s_params.mosfets[s_mosfet_idx].ext_control != 0 ||
                                     s_mosfet_remaining[s_mosfet_idx] == 0U)) {
            s_mosfet_idx++;
        }
        if (s_mosfet_idx >= 2U) {
            s_mosfet_phase = TEST_SEQ_PHASE_DONE;
            return;
        }
        io_state_t next = *io_get();
        next.mux[s_mosfet_idx] = MUX_INT;
        next.mosfet[s_mosfet_idx] = true;
        io_apply(&next);
        if (s_mosfet_remaining[s_mosfet_idx] > 0U) {
            s_mosfet_remaining[s_mosfet_idx]--;
        }
        s_mosfet_deadline_ms = now_ms + clamp_delay_ms(s_params.mosfets[s_mosfet_idx].ton_ms);
        s_mosfet_phase = TEST_SEQ_PHASE_OFF;
        return;
    }

    if (s_mosfet_phase == TEST_SEQ_PHASE_OFF) {
        io_state_t next = *io_get();
        next.mosfet[s_mosfet_idx] = false;
        io_apply(&next);
        s_mosfet_deadline_ms = now_ms + clamp_delay_ms(s_params.mosfets[s_mosfet_idx].toff_ms);
        if (s_mosfet_remaining[s_mosfet_idx] == 0U) {
            s_mosfet_idx++;
        }
        s_mosfet_phase = TEST_SEQ_PHASE_ON;
        return;
    }
}

bool test_seq_tick(uint32_t now_ms)
{
    if (s_state == TEST_SEQ_IDLE) {
        return false;
    }
    if (s_state == TEST_SEQ_DONE) {
        return true;
    }

    test_seq_step_relays(now_ms);
    test_seq_step_mosfets(now_ms);

    if (s_relay_phase == TEST_SEQ_PHASE_DONE && s_mosfet_phase == TEST_SEQ_PHASE_DONE) {
        s_state = TEST_SEQ_DONE;
        io_safe_off();
        return true;
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

uint32_t test_seq_get_relay_initial(uint8_t index)
{
    if (index >= 4U) {
        return 0;
    }
    return s_relay_initial[index];
}

uint32_t test_seq_get_relay_imax(uint8_t index)
{
    if (index >= 4U) {
        return 0;
    }
    return (uint32_t)((s_params.relays[index].imax_ma > 0) ? s_params.relays[index].imax_ma : 0);
}

uint32_t test_seq_get_relay_ton_ms(uint8_t index)
{
    if (index >= 4U) {
        return 0U;
    }
    return (s_params.relays[index].ton_ms > 0) ? (uint32_t)s_params.relays[index].ton_ms : 0U;
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

uint8_t test_seq_get_mosfet_ext_control(uint8_t index)
{
    if (index >= 2U) {
        return 0U;
    }
    return (s_params.mosfets[index].ext_control != 0) ? 1U : 0U;
}

uint8_t test_seq_get_trigger_enabled(void)
{
    return (s_params.trigger.enable != 0) ? 1U : 0U;
}

uint8_t test_seq_get_trigger_channel(void)
{
    if (s_params.trigger.channel <= 0) {
        return 0U;
    }
    return (uint8_t)s_params.trigger.channel;
}
