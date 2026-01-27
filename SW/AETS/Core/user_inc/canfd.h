/*
 * canfd.h
 *
 *  Created on: Sep 22, 2023
 *      Author: Standa
 */

#ifndef CANFD_H_
#define CANFD_H_
#include <stdint.h>

typedef struct {
    uint32_t timestamp_ms;
    uint32_t relay_remaining[4];
    uint8_t relay_state[4];
    uint32_t relay_current_ma[4];
    uint8_t mosfet_state[2];
    uint8_t mux_state[2];
} cfd_telemetry_t;

extern void cfd_Init(void);
extern void cfd_HandleCommunication(const cfd_telemetry_t *telemetry, uint32_t now_ms);
#endif /* CANFD_H_ */
