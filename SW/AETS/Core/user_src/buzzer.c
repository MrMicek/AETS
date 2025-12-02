/*
 * buzzer.c
 * Non-blocking buzzer driver + patterns for alerts
 */

#include "buzzer.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_tim.h"
#include <math.h>

extern TIM_HandleTypeDef htim5; // TIM5 configured in CubeMX
#define BUZZER_TIM         htim5
#define BUZZER_TIM_CH      TIM_CHANNEL_4

// ===== Utilities =====

static uint32_t GetTIM5ClockHz(void)
{
    // TIM on APB1: if APB1 prescaler != 1, timer clock = 2 * PCLK1
    uint32_t pclk = HAL_RCC_GetPCLK1Freq();
    uint32_t cfgr = RCC->CFGR;
    uint32_t ppre = (cfgr & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos;
    bool div1 = (ppre < 4); // 0xxx => /1
    return div1 ? pclk : (pclk * 2U);
}

// Compute PSC/ARR for desired frequency; try to keep ARR in reasonable range.
// Returns false if cannot compute (freq too high/low for current clock).
static bool pwm_compute(uint32_t timclk, uint32_t freq_hz, uint16_t *out_psc, uint16_t *out_arr)
{
    if (freq_hz == 0 || timclk == 0) return false;

    // Aim for ARR ~ 1000..50000 to keep resolution and avoid very high ISR rates.
    // Try a few PSC candidates to hit the freq.
    uint32_t best_psc = 0, best_arr = 0;
    uint32_t best_err = 0xFFFFFFFF;

    for (uint32_t psc = 1; psc <= 0xFFFF; psc *= 2) {
        uint32_t arr = timclk / (psc * freq_hz);
        if (arr == 0 || arr > 0x10000UL) continue; // ARR must fit 16-bit, non-zero
        uint32_t real = timclk / (psc * arr);
        uint32_t err  = (real > freq_hz) ? (real - freq_hz) : (freq_hz - real);

        // Keep ARR reasonable
        if (arr < 200 || arr > 60000) continue;

        if (err < best_err) {
            best_err = err;
            best_psc = psc;
            best_arr = arr;
            if (err == 0) break;
        }
    }

    if (best_psc == 0) {
        // Fallback: direct compute (may yield small ARR)
        uint32_t psc = (timclk / (freq_hz * 60000UL)) + 1U;
        if (psc > 0xFFFF) return false;
        uint32_t arr = timclk / (psc * freq_hz);
        if (arr == 0 || arr > 0x10000UL) return false;
        best_psc = psc;
        best_arr = arr;
    }

    // HAL expects PSC register value = psc-1; ARR register = arr-1 or arr?
    // We’ll use ARR = best_arr-1 to get exact period counts.
    if (best_arr == 0) return false;

    *out_psc = (uint16_t)(best_psc - 1U);
    *out_arr = (uint16_t)(best_arr - 1U);
    return true;
}

// Safe immediate update of PSC/ARR/CCR
static void pwm_apply(uint16_t psc, uint16_t arr, uint16_t ccr)
{
    // Ensure PWM already started in Init. We re-tune on the fly.
    // For immediate effect: reset counter and generate update event.
    __HAL_TIM_DISABLE(&BUZZER_TIM);
    __HAL_TIM_SET_PRESCALER(&BUZZER_TIM, psc);
    __HAL_TIM_SET_AUTORELOAD(&BUZZER_TIM, arr);
    __HAL_TIM_SET_COMPARE(&BUZZER_TIM, BUZZER_TIM_CH, ccr);
    __HAL_TIM_SET_COUNTER(&BUZZER_TIM, 0);
    __HAL_TIM_ENABLE(&BUZZER_TIM);
    BUZZER_TIM.Instance->EGR = TIM_EGR_UG;
}

// ===== Driver state =====

static bool     s_started = false;
static uint32_t s_beep_until_ms = 0;

// Pattern scheduler
typedef struct { uint16_t freq; uint16_t on_ms; uint16_t gap_ms; } buzz_step_t;

static const buzz_step_t PATTERN_DANGER[]   = { {2000, 300, 200}, {2000, 300, 200}, {2000, 300, 0} };
static const buzz_step_t PATTERN_WARNING[]  = { {1500, 200, 300}, {1500, 200, 0} };
static const buzz_step_t PATTERN_BAD_INPUT[]= { {1000, 100, 0} };
static const buzz_step_t PATTERN_INFO[]     = { {2500,  50, 0} };
static const buzz_step_t PATTERN_STARTUP[]  = { {1800, 150, 100}, {1800, 150, 0} };
static const buzz_step_t PATTERN_SHUTDOWN[] = { {1200, 300, 0} };

typedef struct {
    const buzz_step_t *steps;
    uint8_t count;
} pattern_desc_t;

static const pattern_desc_t pattern_table[] = {
    [BUZZER_NONE]     = { NULL, 0 },
    [BUZZER_DANGER]   = { PATTERN_DANGER,    (uint8_t)(sizeof(PATTERN_DANGER)/sizeof(buzz_step_t)) },
    [BUZZER_WARNING]  = { PATTERN_WARNING,   (uint8_t)(sizeof(PATTERN_WARNING)/sizeof(buzz_step_t)) },
    [BUZZER_BAD_INPUT]= { PATTERN_BAD_INPUT, (uint8_t)(sizeof(PATTERN_BAD_INPUT)/sizeof(buzz_step_t)) },
    [BUZZER_INFO]     = { PATTERN_INFO,      (uint8_t)(sizeof(PATTERN_INFO)/sizeof(buzz_step_t)) },
    [BUZZER_STARTUP]  = { PATTERN_STARTUP,   (uint8_t)(sizeof(PATTERN_STARTUP)/sizeof(buzz_step_t)) },
    [BUZZER_SHUTDOWN] = { PATTERN_SHUTDOWN,  (uint8_t)(sizeof(PATTERN_SHUTDOWN)/sizeof(buzz_step_t)) },
};

static buzzer_pattern_t s_pattern = BUZZER_NONE;
static uint8_t  s_step = 0;
static uint32_t s_step_end_ms = 0;
static bool     s_in_gap = false;

// ===== Public API =====

bool Buzzer_Init(void)
{
    // Start PWM and mute
    if (HAL_TIM_PWM_Start(&BUZZER_TIM, BUZZER_TIM_CH) != HAL_OK) {
        return false;
    }
    __HAL_TIM_SET_COMPARE(&BUZZER_TIM, BUZZER_TIM_CH, 0);
    s_started = true;
    s_pattern = BUZZER_NONE;
    s_beep_until_ms = 0;

    Buzzer_PlayPattern(BUZZER_STARTUP);

    return true;


}

void Buzzer_ToneStart(uint32_t freq_hz, float duty01)
{
    if (!s_started) return;
    if (duty01 < 0.f) duty01 = 0.f;
    if (duty01 > 1.f) duty01 = 1.f;

    uint32_t timclk = GetTIM5ClockHz();
    uint16_t psc, arr;
    if (!pwm_compute(timclk, freq_hz, &psc, &arr)) {
        // Fallback: mute if out of range
        __HAL_TIM_SET_COMPARE(&BUZZER_TIM, BUZZER_TIM_CH, 0);
        return;
    }
    uint32_t period_counts = (uint32_t)arr + 1U;
    uint32_t ccr = (uint32_t)(duty01 * (float)period_counts);
    if (ccr > arr) ccr = arr;

    pwm_apply(psc, arr, (uint16_t)ccr);
}

void Buzzer_ToneStop(void)
{
    if (!s_started) return;
    __HAL_TIM_SET_COMPARE(&BUZZER_TIM, BUZZER_TIM_CH, 0);
}

void Buzzer_BeepNB(uint32_t freq_hz, uint32_t duration_ms)
{
    if (!s_started) return;
    Buzzer_ToneStart(freq_hz, 0.5f);
    s_beep_until_ms = HAL_GetTick() + duration_ms;
}

void Buzzer_PlayPattern(buzzer_pattern_t pattern)
{
    s_pattern = pattern;
    s_step = 0;
    s_in_gap = false;
    s_step_end_ms = 0;

    if (pattern == BUZZER_NONE) {
        Buzzer_ToneStop();
        return;
    }

    // Start first step immediately
    const pattern_desc_t *pd = &pattern_table[pattern];
    if (pd->steps && pd->count > 0) {
        Buzzer_ToneStart(pd->steps[0].freq, 0.5f);
        s_step_end_ms = HAL_GetTick() + pd->steps[0].on_ms;
        s_in_gap = false;
        // Also clear single-beep timer
        s_beep_until_ms = 0;
    } else {
        s_pattern = BUZZER_NONE;
        Buzzer_ToneStop();
    }

    while (1) {
           Buzzer_Process();
           if (!Buzzer_IsActive()) break; // Add this helper
         }
}

void Buzzer_Process(void)
{
    uint32_t now = HAL_GetTick();

    // Handle single non-blocking beep
    if (s_beep_until_ms) {
        if ((int32_t)(now - s_beep_until_ms) >= 0) {
            s_beep_until_ms = 0;
            Buzzer_ToneStop();
        }
    }

    // Handle pattern scheduler
    if (s_pattern != BUZZER_NONE) {
        const pattern_desc_t *pd = &pattern_table[s_pattern];
        if (!pd->steps || pd->count == 0) {
            s_pattern = BUZZER_NONE;
            Buzzer_ToneStop();
            return;
        }

        if (!s_in_gap) {
            // Currently beeping: check if on-time elapsed
            if ((int32_t)(now - s_step_end_ms) >= 0) {
                // Move to gap
                Buzzer_ToneStop();
                uint16_t gap = pd->steps[s_step].gap_ms;
                if (gap == 0) {
                    // Next step or end
                    s_step++;
                    if (s_step >= pd->count) {
                        s_pattern = BUZZER_NONE;
                    } else {
                        // Start next tone immediately
                        Buzzer_ToneStart(pd->steps[s_step].freq, 0.5f);
                        s_step_end_ms = now + pd->steps[s_step].on_ms;
                    }
                } else {
                    s_in_gap = true;
                    s_step_end_ms = now + gap;
                }
            }
        } else {
            // In gap
            if ((int32_t)(now - s_step_end_ms) >= 0) {
                s_in_gap = false;
                // Advance to next step
                s_step++;
                if (s_step >= pd->count) {
                    s_pattern = BUZZER_NONE;
                } else {
                    Buzzer_ToneStart(pd->steps[s_step].freq, 0.5f);
                    s_step_end_ms = now + pd->steps[s_step].on_ms;
                }
            }
        }
    }
}

// Optional blocking helper for quick tests (don’t use in production)
void Buzzer_BeepBlocking(uint32_t freq_hz, uint32_t ms)
{
    Buzzer_ToneStart(freq_hz, 0.5f);
    HAL_Delay(ms);
    Buzzer_ToneStop();
}


void Buzzer_Test(void)
{
    const buzzer_pattern_t patterns[] = {
        BUZZER_DANGER,
        BUZZER_WARNING,
        BUZZER_BAD_INPUT,
        BUZZER_INFO,
        BUZZER_STARTUP,
        BUZZER_SHUTDOWN
    };


    for (int i = 0; i < (int)(sizeof(patterns)/sizeof(patterns[0])); i++) {
        // Log to console if you have UART/USB CDC

        Buzzer_PlayPattern(patterns[i]);


        HAL_Delay(1000); // short gap between patterns
    }

}

bool Buzzer_IsActive(void)
{
    return (s_pattern != BUZZER_NONE) || (s_beep_until_ms != 0);
}

