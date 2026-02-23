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
static test_seq_phase_t s_relay_phase[4];
static test_seq_phase_t s_mosfet_phase[2];
static uint32_t s_relay_start_ms[4];
static uint32_t s_mosfet_start_ms[2];

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
    for (uint8_t i = 0; i < 4U; ++i) {
        if (s_params.relays[i].enabled != 0 && s_relay_remaining[i] > 0U) {
            s_relay_phase[i] = TEST_SEQ_PHASE_ON;
            s_relay_start_ms[i] = now_ms;
        } else {
            s_relay_phase[i] = TEST_SEQ_PHASE_DONE;
        }
    }
    for (uint8_t i = 0; i < 2U; ++i) {
        if (s_params.mosfets[i].enabled != 0 && s_params.mosfets[i].ext_control == 0 &&
            s_mosfet_remaining[i] > 0U) {
            s_mosfet_phase[i] = TEST_SEQ_PHASE_ON;
            s_mosfet_start_ms[i] = now_ms;
        } else {
            s_mosfet_phase[i] = TEST_SEQ_PHASE_DONE;
        }
    }
}

void test_seq_stop(void)
{
    s_state = TEST_SEQ_IDLE;
    for (uint8_t i = 0; i < 4U; ++i) {
        s_relay_phase[i] = TEST_SEQ_PHASE_DONE;
    }
    for (uint8_t i = 0; i < 2U; ++i) {
        s_mosfet_phase[i] = TEST_SEQ_PHASE_DONE;
    }
    io_safe_off();
}

static void test_seq_step_relays(uint32_t now_ms)
{
    for (uint8_t i = 0; i < 4U; ++i) {
        if (s_relay_phase[i] == TEST_SEQ_PHASE_DONE) {
            continue;
        }

        // Determine duration for the current phase
        uint32_t duration = 0;
        if (s_relay_phase[i] == TEST_SEQ_PHASE_ON) {
            // waiting OFF-time before turning ON
            duration = clamp_delay_ms(s_params.relays[i].toff_ms);
        } else {
            // waiting ON-time before turning OFF
            duration = clamp_delay_ms(s_params.relays[i].ton_ms);
        }

        // Wait until timeout expires
        if ((now_ms - s_relay_start_ms[i]) < duration) {
            continue;
        }

        // Timeout expired -> switch state
        if (s_relay_phase[i] == TEST_SEQ_PHASE_ON) {
            if (s_params.relays[i].enabled == 0 || s_relay_remaining[i] == 0U) {
                s_relay_phase[i] = TEST_SEQ_PHASE_DONE;
                continue;
            }

            io_state_t next = *io_get();
            next.relays[i] = true;
            io_apply(&next);

            if (s_relay_remaining[i] > 0U) {
                s_relay_remaining[i]--;
            }

            s_relay_start_ms[i] = now_ms;
            s_relay_phase[i] = TEST_SEQ_PHASE_OFF;

        } else { // OFF phase
            io_state_t next = *io_get();
            next.relays[i] = false;
            io_apply(&next);

            s_relay_start_ms[i] = now_ms;
            s_relay_phase[i] = (s_relay_remaining[i] == 0U) ? TEST_SEQ_PHASE_DONE
                                                            : TEST_SEQ_PHASE_ON;
        }
    }
}


static void test_seq_step_mosfets(uint32_t now_ms)
{
    for (uint8_t i = 0; i < 2U; ++i) {
        if (s_mosfet_phase[i] == TEST_SEQ_PHASE_DONE) {
            continue;
        }

        /* LOGIC:
           If Phase is ON, we are waiting for the previous TOFF to complete.
           If Phase is OFF, we are waiting for the previous TON to complete.
        */
        uint32_t duration = 0;
        if (s_mosfet_phase[i] == TEST_SEQ_PHASE_ON) {
            duration = clamp_delay_ms(s_params.mosfets[i].toff_ms);
        } else {
            duration = clamp_delay_ms(s_params.mosfets[i].ton_ms);
        }

        /* Safe Wrap-Around Check */
        if ((now_ms - s_mosfet_start_ms[i]) < duration) {
            continue;
        }

        /* Timer Elapsed - Execute Transition */
        if (s_mosfet_phase[i] == TEST_SEQ_PHASE_ON) {
            // Check run conditions
            if (s_params.mosfets[i].enabled == 0 || s_params.mosfets[i].ext_control != 0 ||
                s_mosfet_remaining[i] == 0U) {
                s_mosfet_phase[i] = TEST_SEQ_PHASE_DONE;
                continue;
            }

            // ACTION: Turn ON
            io_state_t next = *io_get();
            next.mux[i] = MUX_INT;
            next.mosfet[i] = true;
            io_apply(&next);

            // Decrement
            if (s_mosfet_remaining[i] > 0U) {
                s_mosfet_remaining[i]--;
            }

            // Mark start of ON time (wait for TON)
            s_mosfet_start_ms[i] = now_ms;
            s_mosfet_phase[i] = TEST_SEQ_PHASE_OFF;
        }
        else if (s_mosfet_phase[i] == TEST_SEQ_PHASE_OFF) {
            // ACTION: Turn OFF
            io_state_t next = *io_get();
            next.mosfet[i] = false;
            io_apply(&next);

            // Mark start of OFF time (wait for TOFF)
            s_mosfet_start_ms[i] = now_ms;

            if (s_mosfet_remaining[i] == 0U) {
                s_mosfet_phase[i] = TEST_SEQ_PHASE_DONE;
            } else {
                s_mosfet_phase[i] = TEST_SEQ_PHASE_ON;
            }
        }
    }
}

static uint8_t test_seq_all_done(void)
{
    for (uint8_t i = 0; i < 4U; ++i) {
        if (s_relay_phase[i] != TEST_SEQ_PHASE_DONE) {
            return 0U;
        }
    }
    for (uint8_t i = 0; i < 2U; ++i) {
        if (s_mosfet_phase[i] != TEST_SEQ_PHASE_DONE) {
            return 0U;
        }
    }
    return 1U;
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

    if (test_seq_all_done()) {
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
