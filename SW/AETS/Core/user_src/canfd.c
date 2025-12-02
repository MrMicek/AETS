
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

#define CFD_SUPERLOOP_TX_PERIOD_MS  500
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

static int16_t CurrentCh1_mA = 1;
static int16_t CurrentCh2_mA = 2;
static int16_t CurrentCh3_mA = 3;
static int16_t CurrentCh4_mA = 4;

static uint16_t Relay1_Count = 5;
static uint16_t Relay2_Count = 6;
static uint16_t Relay3_Count = 7;
static uint16_t Relay4_Count = 8;

static uint8_t Relay1_State = 1;
static uint8_t Relay2_State = 0;
static uint8_t Relay3_State = 1;
static uint8_t Relay4_State = 0;

static uint8_t Relay1_Alive = 0;
static uint8_t Relay2_Alive = 1;
static uint8_t Relay3_Alive = 0;
static uint8_t Relay4_Alive = 1;

static uint8_t MUX1_State = 1;
static uint8_t MUX2_State = 0;

static uint8_t MOSFET1_State = 0;
static uint8_t MOSFET2_State = 1;

static uint8_t UpCounter = 0;

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

void UpdateTxBufferForCan_Msg1(void) {
    memset(TxData, 0, sizeof(TxData));

    InsertBits(TxData, TXDATABITOFFSET_CURRENT_CH1, 13, CurrentCh1_mA);
    InsertBits(TxData, TXDATABITOFFSET_RELAY1_COUNT_k, 12, Relay1_Count);
    InsertBits(TxData, TXDATABITOFFSET_RELAY1_STATE, 1, Relay1_State);
    InsertBits(TxData, TXDATABITOFFSET_RELAY1_ALIVE, 1, Relay1_Alive);
    InsertBits(TxData, TXDATABITOFFSET_CURRENT_CH2, 13, CurrentCh2_mA);
    InsertBits(TxData, TXDATABITOFFSET_RELAY2_COUNT_k, 12, Relay2_Count);
    InsertBits(TxData, TXDATABITOFFSET_RELAY2_STATE, 1, Relay2_State);
    InsertBits(TxData, TXDATABITOFFSET_RELAY2_ALIVE, 1, Relay2_Alive);
    InsertBits(TxData, TXDATABITOFFSET_MUX1_STATE, 1, MUX1_State);
    InsertBits(TxData, TXDATABITOFFSET_MOSFET1_STATE, 1, MOSFET1_State);
    InsertBits(TxData, TXDATABITOFFSET_UP_COUNTER, 8, UpCounter);

    UpCounter++;
}

void UpdateTxBufferForCan_Msg2(void) {
    memset(TxData, 0, sizeof(TxData));

    InsertBits(TxData, TXDATABITOFFSET_CURRENT_CH1, 13, CurrentCh3_mA);
    InsertBits(TxData, TXDATABITOFFSET_RELAY1_COUNT_k, 12, Relay3_Count);
    InsertBits(TxData, TXDATABITOFFSET_RELAY1_STATE, 1, Relay3_State);
    InsertBits(TxData, TXDATABITOFFSET_RELAY1_ALIVE, 1, Relay3_Alive);
    InsertBits(TxData, TXDATABITOFFSET_CURRENT_CH2, 13, CurrentCh4_mA);
    InsertBits(TxData, TXDATABITOFFSET_RELAY2_COUNT_k, 12, Relay4_Count);
    InsertBits(TxData, TXDATABITOFFSET_RELAY2_STATE, 1, Relay4_State);
    InsertBits(TxData, TXDATABITOFFSET_RELAY2_ALIVE, 1, Relay4_Alive);
    InsertBits(TxData, TXDATABITOFFSET_MUX1_STATE, 1, MUX2_State);
    InsertBits(TxData, TXDATABITOFFSET_MOSFET1_STATE, 1, MOSFET2_State);
    InsertBits(TxData, TXDATABITOFFSET_UP_COUNTER, 8, UpCounter);

    UpCounter++;
}

void cfd_HandleCommunication(void) {
    if (HAL_GetTick() >= LastEvalTimeStamp + CFD_SUPERLOOP_TX_PERIOD_MS) {
        LastEvalTimeStamp = HAL_GetTick();

        HAL_FDCAN_GetProtocolStatus(&hfdcan2, &CanProtStat);
        HAL_FDCAN_GetErrorCounters(&hfdcan2, &CanErrCntrs);

        if (CanProtStat.BusOff) {
            HAL_FDCAN_Stop(&hfdcan2);
            HAL_FDCAN_Start(&hfdcan2);
        }
        TxHeader.Identifier = CFD_OUTPUT_MSG1_ID;
        UpdateTxBufferForCan_Msg1();
        HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, TxData);

        comu_SendF("Msg1: ID=%d Data=", TxHeader.Identifier);
		for (int i = 0; i < 8; i++) {
			comu_SendF("%02X ", TxData[i]);
		}
		comu_SendF("\r\n");

        TxHeader.Identifier = CFD_OUTPUT_MSG2_ID;
        UpdateTxBufferForCan_Msg2();
        HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &TxHeader, TxData);

        comu_SendF("Msg2: ID=%d Data=", TxHeader.Identifier);
        		for (int i = 0; i < 8; i++) {
        			comu_SendF("%02X ", TxData[i]);
        		}
        		comu_SendF("\r\n");
    }
}
