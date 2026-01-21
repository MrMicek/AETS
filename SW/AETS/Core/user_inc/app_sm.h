#ifndef USER_INC_APP_SM_H_
#define USER_INC_APP_SM_H_

#include <stdint.h>
#include <stdbool.h>
#include "error.h"

typedef enum {
    APP_STATE_BOOT = 0,
    APP_STATE_INIT,
    APP_STATE_MANUAL,
    APP_STATE_REMOTE,
    APP_STATE_TEST,
    APP_STATE_FAULT
} app_state_t;

typedef enum {
    APP_TEST_IDLE = 0,
    APP_TEST_RUNNING,
    APP_TEST_ABORTING
} app_test_state_t;

typedef enum {
    APP_EVT_NONE = 0,
    APP_EVT_GOTO_INIT,
    APP_EVT_INIT_DONE,
    APP_EVT_USB_CONNECTED,
    APP_EVT_USB_DISCONNECTED,
    APP_EVT_CMD_MODE_MANUAL,
    APP_EVT_CMD_MODE_REMOTE,
    APP_EVT_CMD_MODE_TEST,
    APP_EVT_CMD_OUTPUT_SET,
    APP_EVT_TEST_START,
    APP_EVT_TEST_STOP,
    APP_EVT_TEST_DONE,
    APP_EVT_TEST_FAIL,
    APP_EVT_TEST_EXIT,
    APP_EVT_FAULT,
    APP_EVT_CLEAR_FAULT
} app_event_type_t;

typedef struct {
    app_event_type_t type;
    uint32_t a;
    uint32_t b;
} app_event_t;

typedef struct {
    app_state_t state;
    app_state_t return_state;
    app_test_state_t test_state;
    uint32_t fault_code;
    uint32_t last_transition_ms;
} app_status_t;

void app_init(void);
bool app_post_event(app_event_t evt);
void app_tick(uint32_t now_ms);
app_status_t app_get_status(void);
uint32_t app_get_relay_current_ma(uint8_t index);

#endif /* USER_INC_APP_SM_H_ */
