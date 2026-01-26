#include "relay_health_store.h"
#include "eeprom.h"
#include "app_params.h"
#include "test_seq.h"
#include <string.h>
#include <stddef.h>

#define RELAY_HEALTH_VERSION          1U
#define RELAY_HEALTH_SLOT_SIZE        64U
#define RELAY_HEALTH_SLOTS            2U
#define RELAY_HEALTH_BASE_ADDR        0x0300U

typedef struct __attribute__((packed)) {
    uint32_t seq;
    uint32_t version;
    uint32_t set_k[4];
    uint32_t remaining_k[4];
    uint16_t crc;
    uint16_t reserved;
} relay_health_record_t;

static uint32_t s_seq = 0;
static volatile uint8_t s_pending_save = 0;

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
    return RELAY_HEALTH_BASE_ADDR + (slot % RELAY_HEALTH_SLOTS) * RELAY_HEALTH_SLOT_SIZE;
}

static HAL_StatusTypeDef relay_health_save(uint32_t timeout_ms)
{
    relay_health_record_t rec;
    memset(&rec, 0xFF, sizeof(rec));
    rec.seq = s_seq + 1U;
    rec.version = RELAY_HEALTH_VERSION;
    for (uint8_t i = 0; i < 4U; ++i) {
        rec.set_k[i] = (uint32_t)g_app_params.relay_health_set_k[i];
        rec.remaining_k[i] = (uint32_t)g_app_params.relay_health_remaining_k[i];
    }
    rec.crc = crc16_ccitt((const uint8_t *)&rec, offsetof(relay_health_record_t, crc));

    uint32_t addr = slot_addr(rec.seq);
    HAL_StatusTypeDef st = EEPROM_WriteTimeout((uint16_t)addr, (uint8_t *)&rec, sizeof(rec), timeout_ms);
    if (st == HAL_OK) {
        s_seq = rec.seq;
        s_pending_save = 0;
    }
    return st;
}

static HAL_StatusTypeDef relay_health_load_internal(void)
{
    uint32_t best_seq = 0;
    relay_health_record_t rec;
    int found = 0;

    for (uint32_t i = 0; i < RELAY_HEALTH_SLOTS; ++i) {
        uint32_t addr = slot_addr(i + 1U);
        if (EEPROM_Read((uint16_t)addr, (uint8_t *)&rec, sizeof(rec)) != HAL_OK) {
            continue;
        }
        uint16_t crc = crc16_ccitt((const uint8_t *)&rec, offsetof(relay_health_record_t, crc));
        if (crc == rec.crc && rec.version == RELAY_HEALTH_VERSION) {
            if (!found || rec.seq > best_seq) {
                best_seq = rec.seq;
                for (uint8_t j = 0; j < 4U; ++j) {
                    g_app_params.relay_health_set_k[j] = (int)rec.set_k[j];
                    g_app_params.relay_health_remaining_k[j] = (int)rec.remaining_k[j];
                }
                found = 1;
            }
        }
    }

    if (!found) {
        return HAL_ERROR;
    }

    s_seq = best_seq;
    s_pending_save = 0;
    return HAL_OK;
}

void relay_health_init(void)
{
    EEPROM_Init();
    s_seq = 0;
    s_pending_save = 0;

    if (relay_health_load_internal() != HAL_OK) {
        for (uint8_t i = 0; i < 4U; ++i) {
            g_app_params.relay_health_remaining_k[i] = g_app_params.relay_health_set_k[i];
        }
        relay_health_save(200U);
    }
}

HAL_StatusTypeDef relay_health_save_now(uint32_t timeout_ms)
{
    return relay_health_save(timeout_ms);
}

void relay_health_request_pending(void)
{
    s_pending_save = 1;
}

void relay_health_handle_pending(void)
{
    if (s_pending_save == 0U) {
        return;
    }
    relay_health_save(200U);
}

void relay_health_update_from_test(void)
{
    uint8_t any_enabled = 0U;
    for (uint8_t i = 0; i < 4U; ++i) {
        if (!test_seq_relay_is_enabled(i)) {
            continue;
        }
        any_enabled = 1U;
        uint32_t initial = test_seq_get_relay_initial(i);
        uint32_t remaining = test_seq_get_relay_remaining(i);
        if (initial > remaining) {
            uint32_t delta = initial - remaining;
            uint32_t delta_k = (delta + 999U) / 1000U;
            int current = g_app_params.relay_health_remaining_k[i];
            if (delta_k >= (uint32_t)current) {
                g_app_params.relay_health_remaining_k[i] = 0;
            } else {
                g_app_params.relay_health_remaining_k[i] = current - (int)delta_k;
            }
        }
    }

    if (any_enabled) {
        relay_health_save(200U);
    }
}
