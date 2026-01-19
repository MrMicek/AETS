/*
 * test_seq.h
 *
 *  Created on: Nov 8, 2025
 */

#ifndef USER_INC_TEST_SEQ_H_
#define USER_INC_TEST_SEQ_H_

#include <stdint.h>
#include <stdbool.h>

void test_seq_start(uint32_t now_ms);
void test_seq_stop(void);
bool test_seq_tick(uint32_t now_ms);
void test_seq_set_params_current(void);
void test_seq_set_params_profile(uint8_t profile_id);
uint32_t test_seq_get_relay_remaining(uint8_t index);
uint32_t test_seq_get_mosfet_remaining(uint8_t index);
bool test_seq_relay_is_enabled(uint8_t index);
bool test_seq_mosfet_is_enabled(uint8_t index);

#endif /* USER_INC_TEST_SEQ_H_ */
