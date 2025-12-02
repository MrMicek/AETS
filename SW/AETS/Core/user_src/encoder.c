/*
 * encoder.c
 *
 *  Created on: Oct 13, 2025
 *      Author: uiv10467
 */

#include "encoder.h"
#include "comuser.h"
#include "main.h"
#include "tim.h"
#include "stm32g4xx_hal.h"
#include <stdlib.h>


// Tunables: depends on how TIM encoder mode is configured (1x/2x/4x edges)
#define ENCODER_STEP_COUNTS   4   // counts per logical "step" (often 2 or 4)
#define ENCODER_DETENT_THRESH ENCODER_STEP_COUNTS

static uint16_t prev_cnt = 0;
static int32_t  accum = 0;
static uint8_t  initialized = 0;




void Encoder_init_once(void) {
    if (!initialized) {
        prev_cnt = __HAL_TIM_GET_COUNTER(&htim1);
        accum = 0;
        initialized = 1;


    }
}



        // Returns ONE event per call: UP / DOWN / NONE
EncoderDirection_Td Encoder_read(void)
        {
            Encoder_init_once();

            uint16_t cur = __HAL_TIM_GET_COUNTER(&htim1);

            // Compute signed delta with wrap, leveraging 16-bit modulo:
            // (uint16_t) subtraction wraps, cast to int16_t yields shortest signed delta
            int16_t d16 = (int16_t)(cur - prev_cnt);
            prev_cnt = cur;

            // Accumulate motion
            accum += d16;

            // Emit at integral steps (positive = DOWN, negative = UP according to your sense)
            if (accum >= ENCODER_DETENT_THRESH) {
                accum -= ENCODER_DETENT_THRESH;
                return DOWN;
            } else if (accum <= -ENCODER_DETENT_THRESH) {
                accum += ENCODER_DETENT_THRESH;
                return UP;
            }

            return NONE;
        }

