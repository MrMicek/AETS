#include "encoder.h"
#include "menu.h"
#include "main.h"
#include "stm32g4xx_hal.h"
#include "tim.h"
#include "display.h"
#include "app_menu.h"
#include <stdio.h>
#include "buzzer.h"
#include <string.h>

extern TIM_HandleTypeDef htim1;

static volatile uint8_t g_encoder_pressed = 0;
static uint32_t g_last_press_ms = 0;

extern uint8_t s_edit_mode;
extern int s_edit_value;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == ENCODER_SW_Pin) {
        uint32_t now = HAL_GetTick();
        if (now - g_last_press_ms > 50) { // 50 ms debounce
            g_last_press_ms = now;
            g_encoder_pressed = 1;
        }
    }
}

// Call this regularly from main loop to update the menu
void menu_poll(Menu *menu) {
    EncoderDirection_Td dir = Encoder_read();


    if (s_edit_mode) {
        const MenuItem *it = &menu_get_active()->items[menu_get_active()->selected];

        if (it->max == 1 && it->min == 0) {
            // Boolean toggle
            if (dir == UP || dir == DOWN) {
                s_edit_value = !s_edit_value; // Toggle ON/OFF
            }
        } else {
            // Numeric edit
            if (dir == UP) s_edit_value++;
            if (dir == DOWN) s_edit_value--;
            if (s_edit_value < it->min) s_edit_value = it->min;
            if (s_edit_value > it->max) s_edit_value = it->max;
        }

        // Update display
        char buf[LCD_COLS + 1];
        if (it->max == 1 && it->min == 0) {
            const char *state = s_edit_value ? "ON" : "OFF";
            int w = snprintf(buf, sizeof(buf), "( %s )", state);
            memset(buf + w, ' ', LCD_COLS - w);
            buf[LCD_COLS] = '\0';
        } else {
            int w = snprintf(buf, sizeof(buf), "( %d )", s_edit_value);
            memset(buf + w, ' ', LCD_COLS - w);
            buf[LCD_COLS] = '\0';
        }
        oled_write_line_full(2, buf);

        if (g_encoder_pressed) {
            g_encoder_pressed = 0;
            *(it->value_ptr) = s_edit_value;
            s_edit_mode = 0;
            oled_clear();
            Buzzer_PlayPattern(BUZZER_INFO);
            menu_draw_full(menu_get_active());
        }
        return;
    }


    // Normal menu navigation
    if (dir == UP)   menu_move_up(menu);
    if (dir == DOWN) menu_move_down(menu);

    if (g_encoder_pressed) {
        g_encoder_pressed = 0;
        menu_select(menu);
    }
}
