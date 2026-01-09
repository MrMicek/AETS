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

#endif /* USER_INC_APP_MENU_H_ */
