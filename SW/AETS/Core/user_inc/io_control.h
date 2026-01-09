#ifndef USER_INC_IO_CONTROL_H_
#define USER_INC_IO_CONTROL_H_

#include <stdbool.h>
#include <stdint.h>
#include "mux.h"

typedef struct {
    bool relays[4];
    bool mosfet[2];
    mux_sel_t mux[2];
} io_state_t;

void io_init(void);
void io_apply(const io_state_t *desired);
void io_safe_off(void);
const io_state_t* io_get(void);

#endif /* USER_INC_IO_CONTROL_H_ */
