/*
 * display.h
 * Driver for Newhaven NHD-0420CW-AW3 OLED (US2066 controller)
 * SPI serial mode – /CS driven by GPIO, /RES tied HIGH (no pin)
 */

#ifndef USER_INC_DISPLAY_H_
#define USER_INC_DISPLAY_H_

#include "stm32g4xx_hal.h"
#define LCD_COLS 20
#define LCD_ROWS 4
#define SHIFT_CURSOR_RIGHT 0x14
#define SHIFT_CURSOR_LEFT 0x10
#define RETURN_HOME 0x02
#define CURSOR_OFF 0x0C
#define CURSOR_ON 0x0E
#define CLEAR_DISPLAY 0x01

void CS_L(void);
void CS_H(void);
void RST_L(void);
void RST_H(void);

void spi_tx1(uint8_t b);

void us2066_expand_and_tx(uint8_t v);

 void us2066_start_cmd(void);
void us2066_start_data(void);
void oled_init(void);
void oled_demo(void);
void oled_reset(void);
 void oled_cmd(uint8_t c);
void oled_dat(uint8_t d);
void oled_set_cursor(uint8_t line, uint8_t col);
void oled_puts(const char *s);
void oled_demo(void);
void oled_clear(void);
void oled_write_line_full(uint8_t line, const char *s);
#endif /* USER_INC_DISPLAY_H_ */
