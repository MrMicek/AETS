/*
 * menu.h
 *
 *  Created on: Nov 6, 2025
 *      Author: uiv10467
 */

#ifndef USER_INC_MENU_H_
#define USER_INC_MENU_H_



#include <stdint.h>

typedef struct Menu Menu;
typedef void (*MenuAction)(void);

typedef struct MenuItem {
    const char *label;
    MenuAction  on_select;  // Action to run (if not NULL)
    Menu       *submenu;    // Submenu to enter (if not NULL)
    int        *value_ptr;   // Editable value (if not NULL)
    int         min;
    int         max;
    int  *readonly_ptr;
} MenuItem;

struct Menu {
    const MenuItem *items;
    uint8_t  count;

    int8_t   selected;
    int8_t   top;

    int8_t   last_drawn_selected;
    int8_t   last_drawn_top;

    Menu    *parent;        // Parent menu to return to
    uint8_t  flags;         // e.g. MENU_FLAG_WRAP
};

#define MENU_FLAG_WRAP   (1u << 0)

// Core init/navigation
void menu_init(Menu *m, const MenuItem *items, uint8_t count);
void menu_move_up(Menu *m);
void menu_move_down(Menu *m);
void menu_select(Menu *m);

// Behavior & context
void menu_set_wrap(Menu *m, uint8_t wrap);  // 1 = wrap, 0 = clamp
void menu_enter(Menu *parent, Menu *child);  // enter submenu
void menu_back(void);                         // go back to parent

// Active menu management (for encoder glue)
void  menu_set_active(Menu *m);
Menu* menu_get_active(void);
void menu_draw_full(Menu *m);


#endif /* USER_INC_MENU_H_ */
