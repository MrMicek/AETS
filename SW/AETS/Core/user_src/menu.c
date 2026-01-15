#include "menu.h"
#include "display.h"
#include <string.h>
#include <stdio.h>
#include "buzzer.h"
#include "app_params.h"



static Menu *s_active = NULL; // currently active menu

uint8_t s_edit_mode = 0; // 0 = normal, 1 = editing
int s_edit_value = 0;    // temporary value while editing
uint8_t s_edit_digit_index = 0;
uint8_t s_edit_digits[MENU_EDIT_DIGITS];

static void menu_edit_digits_from_value(int value)
{
    if (value < 0) value = 0;
    if (value > 999999) value = 999999;

    for (int i = MENU_EDIT_DIGITS - 1; i >= 0; --i) {
        s_edit_digits[i] = (uint8_t)(value % 10);
        value /= 10;
    }
}

static int menu_edit_value_from_digits(void)
{
    int value = 0;
    for (int i = 0; i < MENU_EDIT_DIGITS; ++i) {
        value = (value * 10) + s_edit_digits[i];
    }
    return value;
}

void menu_set_active(Menu *m) { s_active = m; }
Menu* menu_get_active(void)   { return s_active; }

void menu_set_wrap(Menu *m, uint8_t wrap) {
    if (wrap) m->flags |= MENU_FLAG_WRAP;
    else      m->flags &= (uint8_t)~MENU_FLAG_WRAP;
}

void menu_draw_full(Menu *m) {
    for (uint8_t vis = 0; vis < LCD_ROWS; ++vis) {
        int idx = m->top + vis;
        char line[LCD_COLS + 1];

        if (idx >= 0 && idx < m->count) {
            const MenuItem *item = &m->items[idx];
            const char *label = item->label ? item->label : "";
            int selected = (idx == m->selected);

            int written;

            if (item->value_ptr) {
                if (item->max == 1 && item->min == 0) {
                    // Boolean option
                    const char *state = (*(item->value_ptr)) ? "ON" : "OFF";
                    written = snprintf(line, sizeof(line), "%c %-12s %s",
                                       selected ? 0xDF : ' ', label, state);
                } else {
                    // Numeric option
                    written = snprintf(line, sizeof(line), "%c %-12s %d",
                                       selected ? 0xDF : ' ', label, *(item->value_ptr));
                }
            }
            else if(item->readonly_ptr)
			{
				// Readonly numeric option
				written = snprintf(line, sizeof(line), "%c %-12s %d",
						selected ? 0xDF : ' ', label, *(item->readonly_ptr));
			}
            else {
                written = snprintf(line, sizeof(line), "%c %s",
                                   selected ? 0xDF : ' ', label);
            }


            if (written < 0) written = 0;
            if (written > LCD_COLS) written = LCD_COLS;
            memset(line + written, ' ', LCD_COLS - written);
            line[LCD_COLS] = '\0';
        } else {
            memset(line, ' ', LCD_COLS);
            line[LCD_COLS] = '\0';
        }
        oled_write_line_full(vis + 1, line);
    }

    m->last_drawn_selected = m->selected;
    m->last_drawn_top      = m->top;
}

void menu_draw_edit_value(const MenuItem *it)
{
    char line2[LCD_COLS + 1];
    char line3[LCD_COLS + 1];
    char line4[LCD_COLS + 1];

    memset(line2, ' ', LCD_COLS);
    memset(line3, ' ', LCD_COLS);
    memset(line4, ' ', LCD_COLS);
    line2[LCD_COLS] = '\0';
    line3[LCD_COLS] = '\0';
    line4[LCD_COLS] = '\0';

    if (it->max == 1 && it->min == 0) {
        const char *state = s_edit_value ? "ON" : "OFF";
        int w2 = snprintf(line2, sizeof(line2), "( %s )", state);
        if (w2 < 0) w2 = 0;
        if (w2 > LCD_COLS) w2 = LCD_COLS;
        memset(line2 + w2, ' ', LCD_COLS - w2);
    } else {
        int pos = 0;
        for (int i = 0; i < MENU_EDIT_DIGITS; ++i) {
            if (i == 3) {
                line2[pos++] = ' ';
            }
            line2[pos++] = (char)('0' + s_edit_digits[i]);
        }
        if (s_edit_digit_index < MENU_EDIT_DIGITS) {
            int cursor_pos = s_edit_digit_index + (s_edit_digit_index >= 3 ? 1 : 0);
            line3[cursor_pos] = 0xDE;
        }
    }

    oled_write_line_full(2, line2);
    oled_write_line_full(3, line3);
    oled_write_line_full(4, line4);
}

static void menu_draw_delta(Menu *m) {
    if (m->last_drawn_top != m->top || m->last_drawn_selected < 0) {
        menu_draw_full(m);
        return;
    }

    int8_t old_sel = m->last_drawn_selected;
    int8_t new_sel = m->selected;

    for (int pass = 0; pass < 2; ++pass) {
        int idx = (pass == 0) ? old_sel : new_sel;
        if (idx < 0 || idx >= m->count) continue;
        int vis = idx - m->top;
        if (vis < 0 || vis >= LCD_ROWS) continue;

        const MenuItem *item = &m->items[idx];
        const char *label = item->label ? item->label : "";
        char line[LCD_COLS + 1];

        int selected = (idx == new_sel);
        int written;

        if (item->value_ptr) {
            if (item->max == 1 && item->min == 0) {
                // Boolean option
                const char *state = (*(item->value_ptr)) ? "ON" : "OFF";
                written = snprintf(line, sizeof(line), "%c %-12s %s",
                                   selected ? 0xDF : ' ', label, state);
            } else {
                // Numeric option
                written = snprintf(line, sizeof(line), "%c %-12s %d",
                                   selected ? 0xDF : ' ', label, *(item->value_ptr));
            }
        }  else if(item->readonly_ptr)
		{
			// Readonly numeric option
			written = snprintf(line, sizeof(line), "%c %-12s %d",
					selected ? 0xDF : ' ', label, *(item->readonly_ptr));
		}
        else {
            written = snprintf(line, sizeof(line), "%c %s",
                               selected ? 0xDF : ' ', label);
        }


        if (written < 0) written = 0;
        if (written > LCD_COLS) written = LCD_COLS;
        memset(line + written, ' ', LCD_COLS - written);
        line[LCD_COLS] = '\0';

        oled_write_line_full(vis + 1, line);
    }

    m->last_drawn_selected = m->selected;
    m->last_drawn_top      = m->top;
}

void menu_init(Menu *m, const MenuItem *items, uint8_t count) {
    m->items = items;
    m->count = count;
    m->selected = 0;
    m->top = 0;
    m->last_drawn_selected = -1;
    m->last_drawn_top      = -1;
    m->parent = NULL;
    m->flags = MENU_FLAG_WRAP; // default: wrap in top-level menu

    oled_clear();
    menu_draw_full(m);

    if (s_active == NULL) {
        s_active = m;
    }
}

void menu_move_up(Menu *m) {
    if (m->count == 0) return;

    if (m->selected > 0) {
        m->selected--;
        if (m->selected < m->top) {
            m->top = m->selected; // scroll up
        }
    } else {
        if (m->flags & MENU_FLAG_WRAP) {
            m->selected = m->count - 1;
            m->top = (m->count > LCD_ROWS) ? (m->count - LCD_ROWS) : 0;
        } else {
            m->top = 0;
        }
    }
    menu_draw_delta(m);
}

void menu_move_down(Menu *m) {
    if (m->count == 0) return;

    if (m->selected < m->count - 1) {
        m->selected++;
        if (m->selected >= m->top + LCD_ROWS) {
            m->top = m->selected - (LCD_ROWS - 1);
        }
    } else {
        if (m->flags & MENU_FLAG_WRAP) {
            m->selected = 0;
            m->top = 0;
        } else {
            if (m->count > LCD_ROWS) {
                int newTop = m->count - LCD_ROWS;
                if (m->top != newTop) {
                    m->top = newTop;
                }
            }
        }
    }
    menu_draw_delta(m);
}

void menu_enter(Menu *parent, Menu *child) {
    if (!child) return;
    child->parent = parent;
    child->selected = 0;
    child->top = 0;
    child->last_drawn_selected = -1;
    child->last_drawn_top = -1;

    menu_set_wrap(child, 0);
    s_active = child;

    oled_clear();
    menu_draw_full(child);
}

void menu_back(void) {
    if (!s_active) return;
    Menu *parent = s_active->parent;
    if (!parent) return;

    s_active = parent;
    oled_clear();
    menu_draw_full(s_active);
}

void menu_select(Menu *m) {
    if (m->count == 0) {
    	(g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_BAD_INPUT) : 0;
    	return;
    }

    const MenuItem *it = &m->items[m->selected];

    // Editable value?

    if (it->value_ptr) {
    	(g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_INFO) : 0;
        s_edit_mode = 1;
        s_edit_value = *(it->value_ptr);
        s_edit_digit_index = 0;
        if (!(it->max == 1 && it->min == 0)) {
            menu_edit_digits_from_value(s_edit_value);
            s_edit_value = menu_edit_value_from_digits();
        }
        oled_clear();

        char line1[LCD_COLS + 1];
        int w1 = snprintf(line1, sizeof(line1), "Edit %s:", it->label);
        memset(line1 + w1, ' ', LCD_COLS - w1);
        line1[LCD_COLS] = '\0';
        oled_write_line_full(1, line1);
        menu_draw_edit_value(it);
        return;
    }


    if (it->submenu) {
    	(g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_INFO) : 0;
        menu_enter(m, it->submenu);
        return;
    }
    if (it->on_select) {
    	(g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_INFO) : 0;
        it->on_select();
        if (menu_get_active() != m) return;
        m->last_drawn_selected = -1;
        oled_clear();
        menu_draw_full(m);
    }
    else
    	(g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_BAD_INPUT) : 0;
}
