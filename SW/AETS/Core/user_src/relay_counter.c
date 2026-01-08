#include "relay_counter.h"
#include "eeprom.h"
#include "comuser.h"
#include <string.h>
#include <stddef.h>

#define RELAY_COUNTER_VERSION            1U
#define RELAY_COUNTER_SLOT_SIZE          64U
#define RELAY_COUNTER_SLOTS              2U
#define RELAY_COUNTER_BASE_ADDR          0x0200U
#define RELAY_COUNTER_FLUSH_PERIOD_MS    10000U
#define RELAY_COUNTER_DIRTY_THRESHOLD    4U

typedef struct __attribute__((packed)) {
    uint32_t seq;
    uint32_t version;
    uint64_t counts[4];
    uint16_t crc;
    uint16_t reserved;
} relay_counter_record_t;

static uint64_t s_counts[4];
static uint32_t s_seq = 0;
static uint32_t s_dirty = 0;
static uint32_t s_last_flush_ms = 0;

static uint16_t crc16_ccitt(const uint8_t *data, uint32_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint32_t i = 0; i < len; ++i) {
        crc ^= ((uint16_t)data[i]) << 8;
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000U) crc = (uint16_t)((crc << 1) ^ 0x1021U);
            else crc <<= 1;
        }
    }
    return crc;
}

static uint32_t slot_addr(uint32_t slot)
{
    return RELAY_COUNTER_BASE_ADDR + (slot % RELAY_COUNTER_SLOTS) * RELAY_COUNTER_SLOT_SIZE;
}

static HAL_StatusTypeDef relay_counter_save(uint32_t timeout_ms)
{
    relay_counter_record_t rec;
    memset(&rec, 0xFF, sizeof(rec));
    rec.seq = s_seq + 1U;
    rec.version = RELAY_COUNTER_VERSION;
    memcpy(rec.counts, s_counts, sizeof(s_counts));
    rec.crc = crc16_ccitt((const uint8_t *)&rec, offsetof(relay_counter_record_t, crc));

    uint32_t addr = slot_addr(rec.seq);
    HAL_StatusTypeDef st = EEPROM_WriteTimeout((uint16_t)addr, (uint8_t *)&rec, sizeof(rec), timeout_ms);
    if (st == HAL_OK) {
        s_seq = rec.seq;
        s_dirty = 0;
        s_last_flush_ms = HAL_GetTick();
    }
    return st;
}

static HAL_StatusTypeDef relay_counter_load_internal(void)
{
    uint32_t best_seq = 0;
    relay_counter_record_t rec;
    int found = 0;

    for (uint32_t i = 0; i < RELAY_COUNTER_SLOTS; ++i) {
        uint32_t addr = slot_addr(i + 1U); // slot index matches seq%slots
        if (EEPROM_Read((uint16_t)addr, (uint8_t *)&rec, sizeof(rec)) != HAL_OK) {
            continue;
        }
        uint16_t crc = crc16_ccitt((const uint8_t *)&rec, offsetof(relay_counter_record_t, crc));
        if (crc == rec.crc && rec.version == RELAY_COUNTER_VERSION) {
            if (!found || rec.seq > best_seq) {
                best_seq = rec.seq;
                memcpy(s_counts, rec.counts, sizeof(s_counts));
                found = 1;
            }
        }
    }

    if (!found) {
        return HAL_ERROR;
    }

    s_seq = best_seq;
    s_last_flush_ms = HAL_GetTick();
    s_dirty = 0;
    return HAL_OK;
}

void relay_counter_init(void)
{
    EEPROM_Init();
    memset(s_counts, 0, sizeof(s_counts));
    s_seq = 0;
    s_dirty = 0;
    s_last_flush_ms = HAL_GetTick();

    if (relay_counter_load_internal() != HAL_OK) {
        s_dirty = 1;
        relay_counter_save(200U);
    }
}

void relay_counter_on_relay_change(uint8_t index, bool old_state, bool new_state)
{
    if (index >= 4U) {
        return;
    }
    if (old_state != new_state) {
        s_counts[index]++;
        s_dirty++;
    }
}

uint64_t relay_counter_get(uint8_t index)
{
    if (index >= 4U) {
        return 0;
    }
    return s_counts[index];
}

void relay_counter_get_all(uint64_t *out_counts, uint8_t max_len)
{
    if (out_counts == NULL) {
        return;
    }
    uint8_t n = (max_len < 4U) ? max_len : 4U;
    for (uint8_t i = 0; i < n; ++i) {
        out_counts[i] = s_counts[i];
    }
}

void relay_counter_reset(void)
{
    memset(s_counts, 0, sizeof(s_counts));
    s_dirty = 1;
    relay_counter_save(200U);
}

HAL_StatusTypeDef relay_counter_save_now(uint32_t timeout_ms)
{
    return relay_counter_save(timeout_ms);
}

HAL_StatusTypeDef relay_counter_load(void)
{
    memset(s_counts, 0, sizeof(s_counts));
    s_seq = 0;
    s_dirty = 0;
    s_last_flush_ms = HAL_GetTick();
    return relay_counter_load_internal();
}

void relay_counter_periodic_flush(uint32_t now_ms)
{
    if (s_dirty == 0) {
        return;
    }
    if ((now_ms - s_last_flush_ms) >= RELAY_COUNTER_FLUSH_PERIOD_MS || s_dirty >= RELAY_COUNTER_DIRTY_THRESHOLD) {
        relay_counter_save(200U);
    }
}

HAL_StatusTypeDef relay_counter_emergency_flush(void)
{
    return relay_counter_save(50U);
}
