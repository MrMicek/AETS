#include "current.h"
#include "utility.h"
#include "math.h"

extern ADC_HandleTypeDef hadc1;

// Short, conservative timeout (ms)
#define ADC_TIMEOUT_MS  5
#define REFERENCE_VOLTAGE  3300
#define RESOLUTION 4095
#define CURRENT_SENSITIVITY 400
#define MINIMUM_CURRENT 0.007f //this can be adjusted based on needs, leaving it at 7mA -> seems to be the lowest it can detect without failure
#define DIVIDER_RATIO 2
#define ZERO_OUTPUT_VOLTAGE_MV 1250

static float counts_per_A = 0.0f;
static uint32_t offset_counts[4];

void Current_Init(void)
{
    // Make sure ADC is idle. (Calibration is optional and not required here.)
    HAL_ADC_Stop(&hadc1);
    Current_Calibrate_All();
}

uint16_t Current_ReadRaw(current_chTd ch)
{
	ADC_ChannelConfTypeDef s = {0};
	 switch (ch) {
	        case current_ch1: s.Channel = ADC_CHANNEL_9; break; // PC3
	        case current_ch2: s.Channel = ADC_CHANNEL_8; break; // PC2
	        case current_ch3: s.Channel = ADC_CHANNEL_7; break;
	        case current_ch4: s.Channel = ADC_CHANNEL_6; break;
	        case vref_ch: s.Channel = ADC_CHANNEL_VREFINT;break;
	        default: return -1;
	    }
	    s.Rank         = ADC_REGULAR_RANK_1;
	    s.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;   // safe, adjust as needed
	    s.SingleDiff   = ADC_SINGLE_ENDED;
	    s.OffsetNumber = ADC_OFFSET_NONE;
	    s.Offset       = 0;

	    HAL_ADC_ConfigChannel(&hadc1, &s);
	    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

	    HAL_ADC_Start(&hadc1);
	    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	    uint32_t v = HAL_ADC_GetValue(&hadc1);
	    HAL_ADC_Stop(&hadc1);
	    return v;
}



uint32_t Current_Read_mA(uint32_t ch){

	uint32_t raw = avg_raw(ch, 64); // light averaging
	float ma = ((int32_t)raw - (int32_t)offset_counts[ch]) / counts_per_A;
	if(ma < MINIMUM_CURRENT)
		ma = 0;
	return ma * 1000;
}


uint32_t Measure_VDDA_mV(void)
{
    // Ensure internal VREFINT path is enabled (CubeMX: ADC Common -> VREFINT ON)
    HAL_Delay(10); // allow VREFINT stabilization (per RM)
    uint32_t vrefint_code = Current_ReadRaw(vref_ch);

    uint16_t vrefint_cal  = *(__IO uint16_t*) VREFINT_CAL_ADDR;   // from your header
    // VREFINT_CAL_VREF is 3000 mV on G4
    uint32_t vdda_mV = (uint32_t)VREFINT_CAL_VREF * (uint32_t)vrefint_cal / vrefint_code;
    return vdda_mV;
}

void Current_Calibrate_All(void)
{
	uint32_t vdda_mV = Measure_VDDA_mV();
	counts_per_A = (4095.0f * CURRENT_SENSITIVITY) / (float)vdda_mV;

	offset_counts[0] = avg_raw(current_ch1, 1024);
	offset_counts[1] = avg_raw(current_ch2, 1024);
	offset_counts[2] = avg_raw(current_ch3, 1024);
	offset_counts[3] = avg_raw(current_ch4, 1024);
}

uint32_t avg_raw(uint32_t ch, int N){
    uint64_t acc = 0;
    for(int i=0;i<N;i++){ acc += Current_ReadRaw(ch); }
    return (uint32_t)(acc / (uint64_t)N);
}
