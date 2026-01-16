/*
 * mux.c
 *
 *  Created on: Oct 2, 2025
 *      Author: user
 */


#include "mux.h"
#include "main.h"


static void mux_write(mux_channel_t ch, mux_sel_t sel)
{
    GPIO_TypeDef *port = NULL;
    uint16_t pin = 0;

    if (ch == MUX_CH1) {
        port = GPIO_MUX1_GPIO_Port;
        pin = GPIO_MUX1_Pin;
    } else if (ch == MUX_CH2) {
        port = GPIO_MUX2_GPIO_Port;
        pin = GPIO_MUX2_Pin;
    } else {
        return;
    }

    HAL_GPIO_WritePin(port, pin, (sel == MUX_INT) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void MUX_Init(void)
{
    /* Default to external control (EXT = 0) on both mux channels */
    mux_write(MUX_CH1, MUX_EXT);
    mux_write(MUX_CH2, MUX_EXT);
}


void MUX_Set(mux_channel_t ch, mux_sel_t sel)
{
    mux_write(ch, sel);
}


void MUX_Test(void)
{
HAL_Delay(500);
MUX_Set(MUX_CH1, MUX_EXT);
MUX_Set(MUX_CH2, MUX_EXT);
HAL_Delay(500);
MUX_Set(MUX_CH1, MUX_INT);
MUX_Set(MUX_CH2, MUX_INT);
}
