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

#endif /* USER_INC_TEST_SEQ_H_ */
