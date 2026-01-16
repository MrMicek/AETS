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
#include "app_params.h"

extern TIM_HandleTypeDef htim1;

static volatile uint8_t g_encoder_pressed = 0;
static uint32_t g_last_press_ms = 0;

extern uint8_t s_edit_mode;
extern int s_edit_value;
extern uint8_t s_edit_digit_index;
extern uint8_t s_edit_digits[MENU_EDIT_DIGITS];

static int menu_digits_to_value(void)
{
    int value = 0;
    for (int i = 0; i < MENU_EDIT_DIGITS; ++i) {
        value = (value * 10) + s_edit_digits[i];
    }
    return value;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == ENCODER_SW_Pin) {
        uint32_t now = HAL_GetTick();
        if (now - g_last_press_ms > 50) { // 50 ms debounce
            g_last_press_ms = now;
            g_encoder_pressed = 1;
        }
    }
}

uint8_t menu_encoder_take_press(void)
{
    if (g_encoder_pressed) {
        g_encoder_pressed = 0;
        return 1;
    }
    return 0;
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
            // Numeric edit (per-digit wrap)
            if (dir == UP || dir == DOWN) {
                int delta = (dir == UP) ? 1 : -1;
                uint8_t digit = s_edit_digits[s_edit_digit_index];
                digit = (uint8_t)((digit + 10 + delta) % 10);
                s_edit_digits[s_edit_digit_index] = digit;
                s_edit_value = menu_digits_to_value();
            }
        }

        // Update display
        menu_draw_edit_value(it);

        if (g_encoder_pressed) {
            g_encoder_pressed = 0;
            if (it->max == 1 && it->min == 0) {
                *(it->value_ptr) = s_edit_value;
                app_menu_on_value_commit(it);
                s_edit_mode = 0;
                oled_clear();
                (g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_INFO) : 0;
                menu_draw_full(menu_get_active());
            } else {
                if (s_edit_digit_index < (MENU_EDIT_DIGITS - 1)) {
                    s_edit_digit_index++;
                    menu_draw_edit_value(it);
                } else {
                    int new_value = menu_digits_to_value();
                    if (new_value < it->min) new_value = it->min;
                    if (new_value > it->max) new_value = it->max;
                    *(it->value_ptr) = new_value;
                    app_menu_on_value_commit(it);
                    s_edit_mode = 0;
                    s_edit_digit_index = 0;
                    oled_clear();
                    (g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_INFO) : 0;
                    menu_draw_full(menu_get_active());
                }
            }
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
