#include "io_control.h"
#include "relay.h"
#include "mosfet.h"
#include "mux.h"
#include "relay_counter.h"
#include "app_params.h"
#include "app_sm.h"
#include "test_seq.h"
#include "trigger.h"
#include <string.h>

static io_state_t s_state;

void io_init(void)
{
    io_state_t safe = {0};
    safe.mux[0] = MUX_INT;
    safe.mux[1] = MUX_INT;
    io_apply(&safe);
}

void io_apply(const io_state_t *desired)
{
    if (desired == NULL) {
        return;
    }

    app_status_t status = app_get_status();
    uint8_t use_test_params = (status.state == APP_STATE_TEST) ? 1U : 0U;
    io_state_t effective = *desired;

    for (uint8_t i = 0; i < 4; ++i) {
        uint8_t relay_enabled = (use_test_params != 0U) ? (test_seq_relay_is_enabled(i) ? 1U : 0U)
                                                        : (g_app_params.relays[i].enabled != 0);
        if (relay_enabled == 0U) {
            effective.relays[i] = false;
        }
    }

    for (uint8_t i = 0; i < 2; ++i) {
        uint8_t mosfet_enabled = (use_test_params != 0U) ? (test_seq_mosfet_is_enabled(i) ? 1U : 0U)
                                                         : (g_app_params.mosfets[i].enabled != 0);
        if (mosfet_enabled == 0U) {
            effective.mosfet[i] = false;
        }

        if (use_test_params != 0U) {
            effective.mux[i] = test_seq_get_mosfet_ext_control(i) ? MUX_EXT : MUX_INT;
        } else if (g_app_params.mosfets[i].ext_control != 0) {
        	effective.mux[i] = MUX_EXT;
        }
        else {
			effective.mux[i] = MUX_INT;
		}
    }

    for (uint8_t i = 0; i < 4; ++i) {
        bool old_state = s_state.relays[i];
        bool new_state = effective.relays[i];
        if (old_state != new_state) {
            Relay_Set(i, new_state);
            relay_counter_on_relay_change(i, old_state, new_state);
            if (!old_state && new_state) {
                uint8_t trigger_enabled = (use_test_params != 0U) ? test_seq_get_trigger_enabled()
                                                                  : (g_app_params.trigger.enable != 0);
                uint8_t trigger_channel = (use_test_params != 0U) ? test_seq_get_trigger_channel()
                                                                  : (uint8_t)g_app_params.trigger.channel;
                if (trigger_enabled != 0U && trigger_channel == (uint8_t)(i + 1U)) {
                    Trigger_Pulse_us(100U);
                }
            }
            s_state.relays[i] = new_state;
        }
    }

    for (uint8_t i = 0; i < 2; ++i) {
        bool new_state = effective.mosfet[i];
        if (s_state.mosfet[i] != new_state) {
            MOSFET_Set((mosfet_id_t)i, new_state);
            s_state.mosfet[i] = new_state;
        }
    }

    for (uint8_t i = 0; i < 2; ++i) {
        mux_sel_t new_sel = effective.mux[i];
        if (s_state.mux[i] != new_sel) {
            MUX_Set((mux_channel_t)i, new_sel);
            s_state.mux[i] = new_sel;
        }
    }
}

void io_safe_off(void)
{
    io_state_t safe = s_state;
    memset(&safe.relays[0], 0, sizeof(safe.relays));
    memset(&safe.mosfet[0], 0, sizeof(safe.mosfet));
    safe.mux[0] = MUX_INT;
    safe.mux[1] = MUX_INT;
    io_apply(&safe);
}

const io_state_t* io_get(void)
{
    return &s_state;
}
