#include "profile_store.h"
#include "eeprom.h"
#include <string.h>
#include <stddef.h>

#define PROFILE_STORE_VERSION    1U
#define PROFILE_STORE_BASE_ADDR  0x0400U
#define PROFILE_STORE_SLOT_SIZE  192U

typedef struct __attribute__((packed)) {
    uint32_t version;
    app_profile_t profile;
    uint16_t crc;
    uint16_t reserved;
} profile_record_t;

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

static uint32_t profile_addr(uint8_t profile_id)
{
    return PROFILE_STORE_BASE_ADDR + (uint32_t)(profile_id - 1U) * PROFILE_STORE_SLOT_SIZE;
}

void profile_store_from_params(app_profile_t *profile, const app_params_t *params)
{
    if (profile == NULL || params == NULL) {
        return;
    }
    memset(profile, 0, sizeof(*profile));
    profile->buzzer_enable = params->buzzer_enable;
    memcpy(&profile->connectivity, &params->connectivity, sizeof(profile->connectivity));
    memcpy(&profile->trigger, &params->trigger, sizeof(profile->trigger));
    memcpy(&profile->relays[0], &params->relays[0], sizeof(profile->relays));
    memcpy(&profile->mosfets[0], &params->mosfets[0], sizeof(profile->mosfets));
}

void profile_store_apply(const app_profile_t *profile, app_params_t *params)
{
    if (profile == NULL || params == NULL) {
        return;
    }
    params->buzzer_enable = profile->buzzer_enable;
    memcpy(&params->connectivity, &profile->connectivity, sizeof(params->connectivity));
    memcpy(&params->trigger, &profile->trigger, sizeof(params->trigger));
    memcpy(&params->relays[0], &profile->relays[0], sizeof(params->relays));
    memcpy(&params->mosfets[0], &profile->mosfets[0], sizeof(params->mosfets));
}

bool profile_store_save(uint8_t profile_id, const app_params_t *params)
{
    if (params == NULL || profile_id < 1U || profile_id > PROFILE_STORE_COUNT) {
        return false;
    }

    profile_record_t rec;
    memset(&rec, 0xFF, sizeof(rec));
    rec.version = PROFILE_STORE_VERSION;
    profile_store_from_params(&rec.profile, params);
    rec.crc = crc16_ccitt((const uint8_t *)&rec, offsetof(profile_record_t, crc));

    uint32_t addr = profile_addr(profile_id);
    HAL_StatusTypeDef st = EEPROM_WriteTimeout((uint16_t)addr, (uint8_t *)&rec, sizeof(rec), 200U);
    return (st == HAL_OK);
}

bool profile_store_load(uint8_t profile_id, app_profile_t *out_profile)
{
    if (out_profile == NULL || profile_id < 1U || profile_id > PROFILE_STORE_COUNT) {
        return false;
    }

    profile_record_t rec;
    uint32_t addr = profile_addr(profile_id);
    if (EEPROM_Read((uint16_t)addr, (uint8_t *)&rec, sizeof(rec)) != HAL_OK) {
        return false;
    }

    uint16_t crc = crc16_ccitt((const uint8_t *)&rec, offsetof(profile_record_t, crc));
    if (crc != rec.crc || rec.version != PROFILE_STORE_VERSION) {
        return false;
    }

    memcpy(out_profile, &rec.profile, sizeof(*out_profile));
    return true;
}
