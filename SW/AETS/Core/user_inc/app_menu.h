/*
 * app_menu.h
 *
 *  Created on: Nov 6, 2025
 *      Author: uiv10467
 */

#ifndef USER_INC_APP_MENU_H_
#define USER_INC_APP_MENU_H_

#include "menu.h"


void app_menu_init(void);
void app_menu_task(void);
void app_menu_on_value_commit(const MenuItem *it);

typedef enum {
    APP_TEST_SCREEN_NONE = 0,
    APP_TEST_SCREEN_START,
    APP_TEST_SCREEN_RUNNING,
    APP_TEST_SCREEN_STOP,
    APP_TEST_SCREEN_OK,
    APP_TEST_SCREEN_ERROR_MAX_CURRENT,
    APP_TEST_SCREEN_ERROR_ZERO_CURRENT,
	APP_TEST_SCREEN_RELAY_COUNT_LOW
} app_test_screen_t;

void app_menu_set_test_screen(app_test_screen_t screen);
app_test_screen_t app_menu_get_test_screen(void);
void app_menu_set_test_fail_relay(uint8_t relay_idx);
void act_test_current(void);
void act_test_profile(uint8_t profile_id);
void app_menu_draw_test_screen(app_test_screen_t screen);


#endif /* USER_INC_APP_MENU_H_ */
