
#include "canfd.h"
#include <stdint.h>
#include "stm32g4xx_hal.h"
#include "main.h"
#include <string.h>
#include <math.h>
#include "comuser.h"
#include "error.h"
#include "canfd.h"

extern FDCAN_HandleTypeDef hfdcan2;

#define CFD_SUPERLOOP_TX_PERIOD_MS  1000
#define CFD_OUTPUT_MSG1_ID  49   // Channel_1_2
#define CFD_OUTPUT_MSG2_ID  50   // Channel_3_4


static uint32_t LastEvalTimeStamp = 0;
FDCAN_ProtocolStatusTypeDef CanProtStat = {0};
FDCAN_ErrorCountersTypeDef CanErrCntrs = {0};
static FDCAN_TxHeaderTypeDef TxHeader = {0};
uint8_t TxData[8] = {0};

#define TXDATABITOFFSET_CURRENT_CH1     0
#define TXDATABITOFFSET_RELAY1_COUNT_k  13
#define TXDATABITOFFSET_RELAY1_STATE    25
#define TXDATABITOFFSET_RELAY1_ALIVE    26
#define TXDATABITOFFSET_CURRENT_CH2     27
#define TXDATABITOFFSET_RELAY2_COUNT_k  40
#define TXDATABITOFFSET_RELAY2_STATE    52
#define TXDATABITOFFSET_RELAY2_ALIVE    53
#define TXDATABITOFFSET_MUX1_STATE      54
#define TXDATABITOFFSET_MOSFET1_STATE   55
#define TXDATABITOFFSET_UP_COUNTER      56

static uint8_t UpCounter = 0;
static uint16_t relay_count_k(uint32_t remaining)
{
    uint32_t count_k = remaining / 1000U;
    if (count_k > 0x0FFFU) {
        count_k = 0x0FFFU;
    }
    return (uint16_t)count_k;
}

static uint16_t clamp_current_ma(uint32_t current_ma)
{
    if (current_ma > 0x1FFFU) {
        current_ma = 0x1FFFU;
    }
    return (uint16_t)current_ma;
}

// Helper: Insert bits into buffer
static void InsertBits(uint8_t *buf, uint32_t bitOffset, uint32_t bitLength, uint32_t value) {
    for (uint32_t i = 0; i < bitLength; i++) {
        uint32_t bitVal = (value >> i) & 0x01;
        uint32_t byteIndex = (bitOffset + i) / 8;
        uint32_t bitIndex = (bitOffset + i) % 8;
        if (bitVal)
            buf[byteIndex] |= (1 << bitIndex);
        else
            buf[byteIndex] &= ~(1 << bitIndex);
    }
}

void cfd_Init(void) {
    TxHeader.Identifier = CFD_OUTPUT_MSG1_ID;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    HAL_FDCAN_Start(&hfdcan2);
    HAL_GPIO_WritePin(CAN_STB_GPIO_Port, CAN_STB_Pin, GPIO_PIN_RESET);
}

static void UpdateTxBufferForCan_Msg1(const cfd_telemetry_t *telemetry) {
    memset(TxData, 0, sizeof(TxData));

    InsertBits(TxData, TXDATABITOFFSET_CURRENT_CH1, 13, clamp_current_ma(telemetry->relay_current_ma[0]));
    InsertBits(TxData, TXDATABITOFFSET_RELAY1_COUNT_k, 12, relay_count_k(telemetry->relay_remaining[0]));
    InsertBits(TxData, TXDATABITOFFSET_RELAY1_STATE, 1, telemetry->relay_state[0] ? 1U : 0U);
    InsertBits(TxData, TXDATABITOFFSET_RELAY1_ALIVE, 1, telemetry->relay_remaining[0] > 0U ? 1U : 0U);
    InsertBits(TxData, TXDATABITOFFSET_CURRENT_CH2, 13, clamp_current_ma(telemetry->relay_current_ma[1]));
    InsertBits(TxData, TXDATABITOFFSET_RELAY2_COUNT_k, 12, relay_count_k(telemetry->relay_remaining[1]));
    InsertBits(TxData, TXDATABITOFFSET_RELAY2_STATE, 1, telemetry->relay_state[1] ? 1U : 0U);
    InsertBits(TxData, TXDATABITOFFSET_RELAY2_ALIVE, 1, telemetry->relay_remaining[1] > 0U ? 1U : 0U);
    InsertBits(TxData, TXDATABITOFFSET_MUX1_STATE, 1, telemetry->mux_state[0] ? 1U : 0U);
    InsertBits(TxData, TXDATABITOFFSET_MOSFET1_STATE, 1, telemetry->mosfet_state[0] ? 1U : 0U);
    InsertBits(TxData, TXDATABITOFFSET_UP_COUNTER, 8, UpCounter);

    UpCounter++;
}

static void UpdateTxBufferForCan_Msg2(const cfd_telemetry_t *telemetry) {
    memset(TxData, 0, sizeof(TxData));

    InsertBits(TxData, TXDATABITOFFSET_CURRENT_CH1, 13, clamp_current_ma(telemetry->relay_current_ma[2]));
    InsertBits(TxData, TXDATABITOFFSET_RELAY1_COUNT_k, 12, relay_count_k(telemetry->relay_remaining[2]));
    InsertBits(TxData, TXDATABITOFFSET_RELAY1_STATE, 1, telemetry->relay_state[2] ? 1U : 0U);
    InsertBits(TxData, TXDATABITOFFSET_RELAY1_ALIVE, 1, telemetry->relay_remaining[2] > 0U ? 1U : 0U);
    InsertBits(TxData, TXDATABITOFFSET_CURRENT_CH2, 13, clamp_current_ma(telemetry->relay_current_ma[3]));
    InsertBits(TxData, TXDATABITOFFSET_RELAY2_COUNT_k, 12, relay_count_k(telemetry->relay_remaining[3]));
    InsertBits(TxData, TXDATABITOFFSET_RELAY2_STATE, 1, telemetry->relay_state[3] ? 1U : 0U);
    InsertBits(TxData, TXDATABITOFFSET_RELAY2_ALIVE, 1, telemetry->relay_remaining[3] > 0U ? 1U : 0U);
    InsertBits(TxData, TXDATABITOFFSET_MUX1_STATE, 1, telemetry->mux_state[1] ? 1U : 0U);
    InsertBits(TxData, TXDATABITOFFSET_MOSFET1_STATE, 1, telemetry->mosfet_state[1] ? 1U : 0U);
    InsertBits(TxData, TXDATABITOFFSET_UP_COUNTER, 8, UpCounter);

    UpCounter++;
}

void cfd_HandleCommunication(const cfd_telemetry_t *telemetry, uint32_t now_ms) {
    if (telemetry == NULL) {
        return;
    }
    if (now_ms >= LastEvalTimeStamp + CFD_SUPERLOOP_TX_PERIOD_MS) {
        LastEvalTimeStamp = now_ms;

        HAL_FDCAN_GetProtocolStatus(&hfdcan2, &CanProtStat);
        HAL_FDCAN_GetErrorCounters(&hfdcan2, &CanErrCntrs);

        if (CanProtStat.BusOff) {
            HAL_FDCAN_Stop(&hfdcan2);
            HAL_FDCAN_Start(&hfdcan2);
        }
        TxHeader.Identifier = CFD_OUTPUT_MSG1_ID;
        UpdateTxBufferForCan_Msg1(telemetry);
        HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, TxData);

        TxHeader.Identifier = CFD_OUTPUT_MSG2_ID;
        UpdateTxBufferForCan_Msg2(telemetry);
        HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, TxData);
    }
}
