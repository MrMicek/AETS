#include "stm32g4xx_hal.h"
#include <stdint.h>

typedef enum {
    current_ch1 = 0, // PC3 -> ADC1_IN9
    current_ch2 = 1, // PC2 -> ADC1_IN8
    current_ch3 = 2, //      -> ADC1_IN7
    current_ch4 = 3, //      -> ADC1_IN6
	vref_ch = 4,
} current_chTd;

// Call once after ADC init (optional)
void Current_Init(void);

// Read one raw 12-bit sample from the selected channel (0..4095).
// Returns 0xFFFF on error.
uint16_t Current_ReadRaw(current_chTd ch);


uint32_t Current_Read_mA(uint32_t ch);

uint32_t Measure_VDDA_mV(void);

void Current_Calibrate_All(void);

uint32_t avg_raw(uint32_t ch, int N);
