#include "app_sm.h"
#include "io_control.h"
#include "relay_counter.h"
#include "comuser.h"
#include "utility.h"
#include "main.h"
#include "app_menu.h"
#include "test_seq.h"
#include "relay_health_store.h"
#include "current.h"
#include <string.h>
#include "app_params.h"

#define APP_EVENT_QUEUE_LEN 16
#define CURRENT_FAULT_STREAK_LIMIT 6U
#define CURRENT_SAMPLE_SETTLE_MS 100U

/*
 * Developer note: application state machine event flow
 * ----------------------------------------------------
 * BOOT is entered after reset; outputs are forced into a safe state via io_safe_off().
 * An internal APP_EVT_GOTO_INIT is queued from app_init() and moves the machine into INIT.
 * INIT performs basic safety (safe outputs) and immediately posts APP_EVT_INIT_DONE to enter MANUAL.
 * MANUAL allows local UI control; REMOTE is USB-driven; TEST runs a sequencer and remembers
 * which state to return to (MANUAL or REMOTE) once APP_EVT_TEST_DONE/STOP is received.
 * Any state transitions to FAULT on APP_EVT_FAULT, forcing outputs off and latching the code.
 * APP_EVT_CLEAR_FAULT returns to INIT for a clean restart. All transitions are non-blocking
 * and are driven by app_post_event() + app_tick(now_ms) from the main loop.
 */

typedef struct {
    app_status_t status;
    app_event_t queue[APP_EVENT_QUEUE_LEN];
    uint8_t head;
    uint8_t tail;
} app_context_t;

static app_context_t s_ctx;
static uint8_t s_current_over_streak[4];
static uint8_t s_current_zero_streak[4];
static uint8_t s_current_fault_posted = 0U;
static uint32_t s_current_last_ma[4];
static uint32_t s_current_pending_at[4];
static uint8_t s_current_pending[4];
static uint8_t s_current_last_state[4];
static uint32_t s_prev_current_ma[4];

static void app_handle_event(app_event_t evt, uint32_t now_ms);
static void app_enter_state(app_state_t new_state, uint32_t now_ms);
static const char* app_state_str(app_state_t st);

void app_init(void)
{
    s_ctx.status.state = APP_STATE_BOOT;
    s_ctx.status.return_state = APP_STATE_MANUAL;
    s_ctx.status.test_state = APP_TEST_IDLE;
    s_ctx.status.fault_code = 0;
    s_ctx.status.last_transition_ms = HAL_GetTick();
    s_ctx.head = 0;
    s_ctx.tail = 0;

    io_safe_off();
    app_post_event((app_event_t){ .type = APP_EVT_GOTO_INIT });
}

bool app_post_event(app_event_t evt)
{
    uint8_t next_head = (uint8_t)((s_ctx.head + 1U) % APP_EVENT_QUEUE_LEN);
    if (next_head == s_ctx.tail) {
        return false; // queue full
    }
    s_ctx.queue[s_ctx.head] = evt;
    s_ctx.head = next_head;
    return true;
}

static bool app_pop_event(app_event_t *out)
{
    if (s_ctx.head == s_ctx.tail) {
        return false;
    }
    if (out) {
        *out = s_ctx.queue[s_ctx.tail];
    }
    s_ctx.tail = (uint8_t)((s_ctx.tail + 1U) % APP_EVENT_QUEUE_LEN);
    return true;
}

void app_tick(uint32_t now_ms)
{
    app_event_t evt;
    while (app_pop_event(&evt)) {
        app_handle_event(evt, now_ms);
    }

    if (s_ctx.status.state == APP_STATE_TEST) {
        if (test_seq_tick(now_ms)) {
            app_post_event((app_event_t){ .type = APP_EVT_TEST_DONE });
        }

        if (s_ctx.status.test_state == APP_TEST_RUNNING) {
            const io_state_t *io = io_get();

            for (uint8_t i = 0; i < 4U; ++i) {
                uint8_t enabled = test_seq_relay_is_enabled(i) ? 1U : 0U;
                uint8_t relay_on = io->relays[i] ? 1U : 0U;

                if (!enabled) {
                    s_current_over_streak[i] = 0U;
                    s_current_zero_streak[i] = 0U;
                    s_current_pending[i] = 0U;
                    s_current_last_state[i] = 0U;
                    s_current_last_ma[i] = 0U;
                    continue;
                }

                // 1. Detect Change
                if (relay_on != s_current_last_state[i]) {
                    s_current_pending[i] = 1U;

                    uint32_t settle_ms = CURRENT_SAMPLE_SETTLE_MS;
                    if (relay_on) { // Optional: shorter settle if Ton is short
                         uint32_t ton_ms = test_seq_get_relay_ton_ms(i);
                         if (ton_ms > 0U && ton_ms < settle_ms) settle_ms = ton_ms;
                    }
                    s_current_pending_at[i] = now_ms + settle_ms;
                    s_current_last_state[i] = relay_on;
                }

                // 2. Check Logic (Runs on settled state for BOTH ON and OFF)
                if (s_current_pending[i] && now_ms >= s_current_pending_at[i]) {
                    s_current_pending[i] = 0U;

                    uint32_t current_ma = Current_Read_mA(i);
                    s_current_last_ma[i] = current_ma; // Update Display
					if (g_app_params.current_monitoring_enabled) {


						// --- CHECK A: OVERCURRENT (Always bad) ---
						uint32_t imax_ma = test_seq_get_relay_imax(i);
						if (imax_ma > 0U &&
							current_ma > imax_ma ||
							s_prev_current_ma[i] > imax_ma) {
							if (s_current_over_streak[i] < CURRENT_FAULT_STREAK_LIMIT) {
								s_current_over_streak[i]++;
							}
						} else {
							s_current_over_streak[i] = 0U;
						}

						// --- CHECK B: ZERO CURRENT (Universal Logic) ---
						// If Current is 0 NOW, AND it was 0 BEFORE...
						// Then it never turned on. That is a failure.
						if (current_ma == 0U && s_prev_current_ma[i] == 0U) {
							if (s_current_zero_streak[i] < CURRENT_FAULT_STREAK_LIMIT)
								s_current_zero_streak[i]++;
						} else {
							// If current is flowing OR previous was flowing, we are good.
							s_current_zero_streak[i] = 0U;
						}

						// Store this reading for the NEXT comparison
						s_prev_current_ma[i] = current_ma;

						// --- TRIGGER FAULT ---
						if (s_current_fault_posted == 0U &&
							(s_current_over_streak[i] >= CURRENT_FAULT_STREAK_LIMIT ||
							 s_current_zero_streak[i] >= CURRENT_FAULT_STREAK_LIMIT)) {

							s_current_fault_posted = 1U;
							app_post_event((app_event_t){
								.type = APP_EVT_TEST_FAIL,
								.a = (s_current_zero_streak[i] >= CURRENT_FAULT_STREAK_LIMIT) ? 2U : 1U
							});
							break;
						}
                    }
                }
            }
        }
    }
}

static void app_handle_event(app_event_t evt, uint32_t now_ms)
{
    switch (evt.type) {
    case APP_EVT_GOTO_INIT:
        app_enter_state(APP_STATE_INIT, now_ms);
        app_post_event((app_event_t){ .type = APP_EVT_INIT_DONE });
        break;

    case APP_EVT_INIT_DONE:
        if (s_ctx.status.state == APP_STATE_INIT) {
            app_enter_state(APP_STATE_MANUAL, now_ms);
        }
        break;

    case APP_EVT_CMD_MODE_MANUAL:
        if (s_ctx.status.state != APP_STATE_FAULT) {
            app_enter_state(APP_STATE_MANUAL, now_ms);
        }
        break;

    case APP_EVT_CMD_MODE_REMOTE:
        if (s_ctx.status.state != APP_STATE_FAULT) {
            app_enter_state(APP_STATE_REMOTE, now_ms);
        }
        break;

    case APP_EVT_CMD_MODE_TEST:
    case APP_EVT_TEST_START:
        if (s_ctx.status.state != APP_STATE_FAULT) {
        	for (int i = 0; i < 4; ++i) {
        	    	    const relay_params_t *rp = &g_app_params.relays[i];
        	    	    int sw_count_k = rp->sw_count_k;

        	    	    if (rp->enabled != 0 && sw_count_k > 0) {
        	    	        if (g_app_params.relay_health_remaining_k[i] < sw_count_k) {
        	    	            app_menu_set_test_fail_relay((uint8_t)(i + 1));
        	    	            app_menu_set_test_screen(APP_TEST_SCREEN_RELAY_COUNT_LOW);
        	    	            return;
        	    	        }
        	    	    }
        	    	}
        	for(int i=0; i<4; i++) {
        	                s_prev_current_ma[i] = Current_Read_mA(i);
        	            }
            s_ctx.status.return_state = (s_ctx.status.state == APP_STATE_REMOTE) ? APP_STATE_REMOTE : APP_STATE_MANUAL;
            app_enter_state(APP_STATE_TEST, now_ms);
        }
        break;

    case APP_EVT_TEST_STOP:
    case APP_EVT_TEST_FAIL:
        if (s_ctx.status.state == APP_STATE_TEST) {
            s_ctx.status.test_state = APP_TEST_ABORTING;
            relay_health_update_from_test();
            test_seq_stop();
            if (evt.type == APP_EVT_TEST_STOP) {
                app_menu_set_test_screen(APP_TEST_SCREEN_STOP);
                comu_SendF("Test Stopped\r\n");
            } else {
                if (evt.a == 2U) {
                    app_menu_set_test_screen(APP_TEST_SCREEN_ERROR_ZERO_CURRENT);
                    comu_SendF("Test Fail: Zero Current\r\n");
                } else {
                    app_menu_set_test_screen(APP_TEST_SCREEN_ERROR_MAX_CURRENT);
                    comu_SendF("Test Fail: OverCurrent\r\n");
                }
            }
        }
        break;

    case APP_EVT_TEST_DONE:
        if (s_ctx.status.state == APP_STATE_TEST) {
            s_ctx.status.test_state = APP_TEST_IDLE;
            relay_health_update_from_test();
            test_seq_stop();
            app_menu_set_test_screen(APP_TEST_SCREEN_OK);
            comu_SendF("Test Done\r\n");
        }
        break;

    case APP_EVT_TEST_EXIT:
        if (s_ctx.status.state == APP_STATE_TEST) {
            s_ctx.status.test_state = APP_TEST_IDLE;
            app_enter_state(s_ctx.status.return_state, now_ms);
        }
        break;

    case APP_EVT_FAULT:
        s_ctx.status.fault_code = evt.a;
        app_enter_state(APP_STATE_FAULT, now_ms);
        break;

    case APP_EVT_CLEAR_FAULT:
        if (s_ctx.status.state == APP_STATE_FAULT) {
            s_ctx.status.fault_code = 0;
            app_enter_state(APP_STATE_INIT, now_ms);
            app_post_event((app_event_t){ .type = APP_EVT_INIT_DONE });
        }
        break;

    default:
        break;
    }
}

static void app_enter_state(app_state_t new_state, uint32_t now_ms)
{
    if (s_ctx.status.state == new_state) {
        return;
    }

    s_ctx.status.state = new_state;
    s_ctx.status.last_transition_ms = now_ms;

    switch (new_state) {
    case APP_STATE_BOOT:
        io_safe_off();
        s_ctx.status.test_state = APP_TEST_IDLE;
        break;

    case APP_STATE_INIT:
        io_safe_off();
        s_ctx.status.test_state = APP_TEST_IDLE;
        break;

    case APP_STATE_MANUAL:
        s_ctx.status.test_state = APP_TEST_IDLE;
        app_menu_set_test_screen(APP_TEST_SCREEN_NONE);
        comu_SendF("evt state %s\r\n", app_state_str(new_state));
        break;

    case APP_STATE_REMOTE:
        s_ctx.status.test_state = APP_TEST_IDLE;
        app_menu_set_test_screen(APP_TEST_SCREEN_NONE);
        comu_SendF("evt state %s\r\n", app_state_str(new_state));
        break;

    case APP_STATE_TEST:
        io_safe_off();
        s_ctx.status.test_state = APP_TEST_RUNNING;
        app_menu_set_test_screen(APP_TEST_SCREEN_RUNNING);
        test_seq_start(now_ms);
        memset(s_current_over_streak, 0, sizeof(s_current_over_streak));
        memset(s_current_zero_streak, 0, sizeof(s_current_zero_streak));
        s_current_fault_posted = 0U;
        memset(s_current_last_ma, 0, sizeof(s_current_last_ma));
        memset(s_current_pending, 0, sizeof(s_current_pending));
        memset(s_current_pending_at, 0, sizeof(s_current_pending_at));
        memset(s_current_last_state, 0, sizeof(s_current_last_state));
        comu_SendF("evt state %s\r\n", app_state_str(new_state));
        break;

    case APP_STATE_FAULT:
        io_safe_off();
        app_menu_set_test_screen(APP_TEST_SCREEN_NONE);
        comu_SendF("evt fault %lu %lu\r\n", (unsigned long)s_ctx.status.fault_code, (unsigned long)now_ms);
        break;

    default:
        break;
    }
}

app_status_t app_get_status(void)
{
    return s_ctx.status;
}

uint32_t app_get_relay_current_ma(uint8_t index)
{
    if (index >= 4U) {
        return 0U;
    }
    return s_current_last_ma[index];
}

static const char* app_state_str(app_state_t st)
{
    switch (st) {
    case APP_STATE_BOOT: return "BOOT";
    case APP_STATE_INIT: return "INIT";
    case APP_STATE_MANUAL: return "MANUAL";
    case APP_STATE_REMOTE: return "REMOTE";
    case APP_STATE_TEST: return "TEST";
    case APP_STATE_FAULT: return "FAULT";
    default: return "?";
    }
}
