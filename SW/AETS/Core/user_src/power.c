
#include "power.h"
#include "error.h"
#include "stm32g4xx_hal.h"
extern ADC_HandleTypeDef hadc2;

// Hard-coded thresholds in ADC counts (12-bit scale)
#define BROWNOUT_LOW_COUNTS   1000
#define BROWNOUT_HIGH_COUNTS  1580

err_Td Power_InitBrownout(void)
{
    // 1) Ensure ADC2 is idle
    HAL_ADCEx_InjectedStop(&hadc2);
    HAL_ADC_Stop_IT(&hadc2);
    HAL_ADC_Stop(&hadc2);

    // 2) Calibrate
    if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
        return err_Td_NotValid;

    // 3) Program AWD window (make sure this Channel == Regular Rank 1 in MX_ADC2_Init)
    ADC_AnalogWDGConfTypeDef cfg = {0};
    cfg.WatchdogNumber = ADC_ANALOGWATCHDOG_1;
    cfg.WatchdogMode   = ADC_ANALOGWATCHDOG_SINGLE_REG;
    cfg.Channel        = ADC_CHANNEL_1;      // <-- adjust to your actual Rank 1 channel
    cfg.ITMode         = ENABLE;
    cfg.LowThreshold   = 1000;               // your hard-coded low
    cfg.HighThreshold  = 1580;               // your hard-coded high
    if (HAL_ADC_AnalogWDGConfig(&hadc2, &cfg) != HAL_OK)
        return err_Td_NotValid;

    // 4) Clear any pending flags BEFORE starting
    __HAL_ADC_CLEAR_FLAG(&hadc2, ADC_FLAG_AWD1 | ADC_FLAG_EOC | ADC_FLAG_EOS | ADC_FLAG_OVR);

    // 5) Start conversions WITHOUT IT (continuous mode must be enabled in .ioc)
    if (HAL_ADC_Start(&hadc2) != HAL_OK)
        return err_Td_NotValid;

    // 6) (Optional) Discard a few conversions to settle the S&H capacitor
    for (int i = 0; i < 3; i++) {
        if (HAL_ADC_PollForConversion(&hadc2, 10) != HAL_OK) break;
        (void)HAL_ADC_GetValue(&hadc2);
    }

    // 7) Clear flags again (just in case the discards set EOC/EOS)
    __HAL_ADC_CLEAR_FLAG(&hadc2, ADC_FLAG_AWD1 | ADC_FLAG_EOC | ADC_FLAG_EOS | ADC_FLAG_OVR);

    // 8) Enable ONLY the AWD interrupt bit
    __HAL_ADC_ENABLE_IT(&hadc2, ADC_IT_AWD1);

    // 9) Enable NVIC for ADC1_2 (ADC2 shares this line)
    HAL_NVIC_SetPriority(ADC1_2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);

    return err_Td_Ok;
}

void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC2) {
        Power_OnBrownout();   // your hook
    }
}

__weak void Power_OnBrownout(void)
{
    /* User hook: do emergency EEPROM save, cut loads, beep, etc. */
}

// Don’t block here. Keep this as a one-time configure function or remove it.
void Power_TestBrownout(void)
{
    // Old version had: while(1) { __WFI(); }  // <-- BLOCKS FOREVER
    // Remove the infinite loop. If you want to idle, do it in main() loop.
}



