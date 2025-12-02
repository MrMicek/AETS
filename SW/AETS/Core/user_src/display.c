/*
 * display.c
 * NHD-0420CW-AW3 SPI driver (US2066 controller)
 * /RES controlled by MCU, /CS under MCU control
 */

#include "display.h"
#include <string.h>
#include "main.h" // for GPIO pin definitions

extern SPI_HandleTypeDef hspi1; // from main.c


#define LCD_COLS 20
#define LCD_ROWS 4



static void oled_write_padded_line(uint8_t line, uint8_t col, const char *s) {
    char buf[LCD_COLS + 1];
    size_t n = strlen(s);
    if (n > LCD_COLS - col) n = LCD_COLS - col;
    memset(buf, ' ', LCD_COLS);            // fill with spaces
    memcpy(buf, s, n);                      // copy as much as fits
    buf[LCD_COLS] = '\0';
    oled_set_cursor(line, col);
    oled_puts(buf);
}


void oled_write_line_full(uint8_t line, const char *s) {
    oled_write_padded_line(line, 0, s);
}


void oled_cursor_off(void) { oled_cmd(0x0C); }
void oled_cursor_on(void)  { oled_cmd(0x0E); }
void oled_cursor_blink_on(void) { oled_cmd(0x0F); }


void CS_L(void){ HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET); }
void CS_H(void){ HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);  }
void RST_L(void){ HAL_GPIO_WritePin(PA15_GPIO_Port, PA15_Pin, GPIO_PIN_RESET); }
void RST_H(void){ HAL_GPIO_WritePin(PA15_GPIO_Port, PA15_Pin, GPIO_PIN_SET);  }


// Send one byte over SPI (8-bit, mode0, MSB first)
void spi_tx1(uint8_t b){HAL_SPI_Transmit(&hspi1, &b, 1, HAL_MAX_DELAY); }

// Expand an 8-bit value into two serial-interface bytes (nibble + 0x0, nibble + 0x0)
void us2066_expand_and_tx(uint8_t v){
	uint8_t hi = (v >> 4);          // D7..D4 then 0000
	uint8_t lo = (v & 0x0F);     // D3..D0 then 0000
	spi_tx1(lo);
	spi_tx1(hi);
}



// Start byte: 0xF8 (cmd) or 0xFA (data)
void us2066_start_cmd(void){ spi_tx1(0x1F); } // 00011111
void us2066_start_data(void){ spi_tx1(0x5F); } // 01011111

void oled_reset(void){
    CS_H();
    RST_L(); HAL_Delay(2);
    RST_H(); HAL_Delay(2); // datasheet: tRES ≥ 2 µs; we give ms margin
}

void oled_cmd(uint8_t c){
    CS_L();
    us2066_start_cmd();
    us2066_expand_and_tx(c);
    CS_H();
}
void oled_dat(uint8_t d){
    CS_L();
    us2066_start_data();
    us2066_expand_and_tx(d);
    CS_H();
}

void oled_init(void){
    oled_reset();

    // REGVDD tied LOW ⇒ LV I/O path; BS[2:0]=000 for serial; D/C# tied LOW per serial mode. :contentReference[oaicite:5]{index=5}
    // Init (US2066/NHD basic sequence):
    oled_cmd(0x2A);
    oled_cmd(0x2A);                 // function set (extended)
    oled_cmd(0x71); oled_dat(0x00); // Function Selection A: disable internal VDD reg (LV I/O)
    oled_cmd(0x28);                 // function set (fundamental)
    oled_cmd(0x08);                 // display OFF
    oled_cmd(0x2A); oled_cmd(0x79);// extended + enable OLED cmds
    oled_cmd(0xD5); oled_cmd(0x70);// clk divide/osc
    oled_cmd(0x78);                 // disable OLED cmds
    oled_cmd(0x09);                 // 4-line (NW=1)
    oled_cmd(0x06);                 // COM/SEG dir (per app note)
    oled_cmd(0x72); oled_dat(0x00);// Function Selection B
    oled_cmd(0x2A); oled_cmd(0x79);// enable OLED cmds
    oled_cmd(0xDA); oled_cmd(0x10);// SEG pins hw cfg
    oled_cmd(0xDC); oled_cmd(0x00);// Function Selection C
    oled_cmd(0x81); oled_cmd(0x7F);// contrast
    oled_cmd(0xD9); oled_cmd(0xF1);// phase length
    oled_cmd(0xDB); oled_cmd(0x40);// VCOMH
    oled_cmd(0x78);                 // disable OLED cmds
    oled_cmd(0x28);                 // fundamental
    oled_cmd(0x01); HAL_Delay(2);  // clear
    oled_cmd(0x80);                // DDRAM addr = 0
    oled_cmd(0x0C);                // display ON, cursor/blink OFF
    oled_set_cursor(1,0); oled_puts("Automated");
    oled_set_cursor(2,0); oled_puts("Electrical");
    oled_set_cursor(3,0); oled_puts("Testing");
    oled_set_cursor(4,0); oled_puts("System         v0.1");
    //initial message

}

void oled_puts(const char *s){ while(*s) oled_dat((uint8_t)*s++); }

void oled_clear(void) {oled_cmd(CLEAR_DISPLAY);}

void oled_set_cursor(uint8_t line, uint8_t col){
    static const uint8_t base[4] = {0x00,0x20,0x40,0x60}; // DDRAM bases for 4×20 (5-dot font) :contentReference[oaicite:6]{index=6}
    if(line<1||line>4) line=1; if(col>19) col=19;
    oled_cmd(0x80 | (base[line-1] + col));
}

void oled_demo(void){
    oled_cmd(0x01); HAL_Delay(2);
    oled_set_cursor(1,5); oled_puts("NHD-0420CW-AW3");
    oled_set_cursor(2,0); oled_puts("US2066 serial");
    oled_set_cursor(3,3); oled_puts("SPI mode 0");
    oled_set_cursor(4,8); oled_puts("Hello!>");
}


