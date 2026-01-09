#include "io_control.h"
#include "relay.h"
#include "mosfet.h"
#include "mux.h"
#include "relay_counter.h"
#include <string.h>

static io_state_t s_state;

void io_init(void)
{
    io_state_t safe = {0};
    safe.mux[0] = MUX_EXT;
    safe.mux[1] = MUX_EXT;
    io_apply(&safe);
}

void io_apply(const io_state_t *desired)
{
    if (desired == NULL) {
        return;
    }

    for (uint8_t i = 0; i < 4; ++i) {
        bool old_state = s_state.relays[i];
        bool new_state = desired->relays[i];
        if (old_state != new_state) {
            Relay_Set(i, new_state);
            relay_counter_on_relay_change(i, old_state, new_state);
            s_state.relays[i] = new_state;
        }
    }

    for (uint8_t i = 0; i < 2; ++i) {
        bool new_state = desired->mosfet[i];
        if (s_state.mosfet[i] != new_state) {
            MOSFET_Set((mosfet_id_t)i, new_state);
            s_state.mosfet[i] = new_state;
        }
    }

    for (uint8_t i = 0; i < 2; ++i) {
        mux_sel_t new_sel = desired->mux[i];
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
    safe.mux[0] = MUX_EXT;
    safe.mux[1] = MUX_EXT;
    io_apply(&safe);
}

const io_state_t* io_get(void)
{
    return &s_state;
}
