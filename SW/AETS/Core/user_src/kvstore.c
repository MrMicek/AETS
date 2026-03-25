#include "kvstore.h"
#include "eeprom.h"
#include "stm32g4xx_hal.h"
#include <string.h>
#include "comuser.h"
#include "relay_health_store.h"

static uint32_t s_counter = 0;
static uint32_t s_seq = 0;
static volatile uint8_t s_pending_save = 0; // set by ISR/hook when brownout detected

#define KV_BASE_ADDR  0x0000
#define KV_SLOTS      2
#define KV_SLOT_SIZE  10 /* seq(4) + counter(4) + crc16(2) = 10 bytes */

// slot layout: [0..3] seq (u32), [4..7] counter (u32), [8..9] crc16 (u16)

static uint16_t crc16_ccitt(const uint8_t *data, uint32_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint32_t i = 0; i < len; ++i) {
        crc ^= ((uint16_t)data[i]) << 8;
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000) crc = (uint16_t)((crc << 1) ^ 0x1021);
            else crc <<= 1;
        }
    }
    return crc;
}

static uint32_t slot_addr(unsigned int idx)
{
    return KV_BASE_ADDR + (idx % KV_SLOTS) * KV_SLOT_SIZE;
}

void KV_Init(void)
{
    EEPROM_Init();

    uint8_t buf[KV_SLOT_SIZE];
    uint32_t best_seq = 0;
    uint32_t best_counter = 0;
    int found = 0;

    for (unsigned int i = 0; i < KV_SLOTS; ++i) {
        uint32_t addr = slot_addr(i);
        if (EEPROM_Read(addr, buf, KV_SLOT_SIZE) != HAL_OK) continue;
        uint32_t seq = 0;
        uint32_t counter = 0;
        uint16_t crc = 0;
        memcpy(&seq, &buf[0], sizeof(seq));
        memcpy(&counter, &buf[4], sizeof(counter));
        memcpy(&crc, &buf[8], sizeof(crc));
        uint16_t calc = crc16_ccitt(buf, 8); // seq + counter
        if (calc == crc) {
            if (!found || seq > best_seq) {
                best_seq = seq;
                best_counter = counter;
                found = 1;
            }
        }
    }

    if (found) {
        s_seq = best_seq;
        s_counter = best_counter;
    } else {
        s_seq = 0;
        s_counter = 0;
        KV_SaveCounter(); // write initial state
    }
}

uint32_t KV_GetCounter(void)
{
    return s_counter;
}

void KV_IncCounter(void)
{
    s_counter++;
}

HAL_StatusTypeDef KV_SaveCounter(void)
{
    // Prepare next sequence and slot
    uint32_t next_seq = s_seq + 1;
    unsigned int slot = next_seq % KV_SLOTS; // alternate
    uint8_t buf[KV_SLOT_SIZE];
    memset(buf, 0xFF, KV_SLOT_SIZE); // fill with 0xFF for clarity
    memcpy(&buf[0], &next_seq, sizeof(next_seq));
    memcpy(&buf[4], &s_counter, sizeof(s_counter));
    uint16_t crc = crc16_ccitt(buf, 8);
    memcpy(&buf[8], &crc, sizeof(crc));

    uint32_t addr = slot_addr(slot);
    uint32_t t0 = HAL_GetTick();
    HAL_StatusTypeDef st = EEPROM_WriteTimeout(addr, buf, KV_SLOT_SIZE, 300);
    uint32_t t1 = HAL_GetTick();
    if (st == HAL_OK) {
        // Update in-memory seq only after successful write
        s_seq = next_seq;
        comu_SendF("KV_SaveCounter OK seq=%lu cnt=%lu dt=%lums\r\n", (unsigned long)s_seq, (unsigned long)s_counter, (unsigned long)(t1 - t0));
    } else {
        comu_SendF("KV_SaveCounter FAIL st=%d seq=%lu cnt=%lu dt=%lums\r\n", (int)st, (unsigned long)next_seq, (unsigned long)s_counter, (unsigned long)(t1 - t0));
    }
    return st;
}

/* Called from main context: handle pending brownout save requested from ISR */
void KV_HandlePending(void)
{
    if (s_pending_save) {
        // Clear flag early to avoid reentry
        s_pending_save = 0;

        uint32_t next_seq = s_seq + 1;
        unsigned int slot = next_seq % KV_SLOTS; // alternate
        uint8_t buf[KV_SLOT_SIZE];
        memset(buf, 0xFF, KV_SLOT_SIZE);
        memcpy(&buf[0], &next_seq, sizeof(next_seq));
        memcpy(&buf[4], &s_counter, sizeof(s_counter));
        uint16_t crc = crc16_ccitt(buf, 8);
        memcpy(&buf[8], &crc, sizeof(crc));
        uint32_t addr = slot_addr(slot);

        uint32_t t0 = HAL_GetTick();
        // Disable interrupts to reduce interference and increase chance of completion
        __disable_irq();
        HAL_StatusTypeDef st = EEPROM_WriteTimeout(addr, buf, KV_SLOT_SIZE, 200);
        if (st == HAL_OK) {
            s_seq = next_seq;
        }
        __enable_irq();
        uint32_t t1 = HAL_GetTick();
        comu_SendF("KV_HandlePending write st=%d seq=%lu cnt=%lu dt=%lums\r\n", (int)st, (unsigned long)next_seq, (unsigned long)s_counter, (unsigned long)(t1 - t0));
    }
}

/* Power_OnBrownout should be very fast: just set pending flag */
void Power_OnBrownout(void)
{
    s_pending_save = 1;
    relay_health_request_pending();

#if KV_IMMEDIATE_ON_BROWNOUT
    // Best-effort immediate short blocking write. This may call HAL I2C APIs from
    // IRQ context and is potentially unsafe on some systems; use only for tests.
    uint32_t next_seq = s_seq + 1;
    unsigned int slot = next_seq % KV_SLOTS; // alternate
    uint8_t buf[KV_SLOT_SIZE];
    memset(buf, 0xFF, KV_SLOT_SIZE);
    memcpy(&buf[0], &next_seq, sizeof(next_seq));
    memcpy(&buf[4], &s_counter, sizeof(s_counter));
    uint16_t crc = crc16_ccitt(buf, 8);
    memcpy(&buf[8], &crc, sizeof(crc));
    uint32_t addr = slot_addr(slot);

    // Disable nested interrupts while doing this very short, best-effort write
    __disable_irq();
    HAL_StatusTypeDef st = EEPROM_WriteTimeout(addr, buf, KV_SLOT_SIZE, 50);
    if (st == HAL_OK) {
        s_seq = next_seq;
    }
    __enable_irq();
#endif
}
