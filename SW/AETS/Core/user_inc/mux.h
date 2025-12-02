/*
 * mux.h
 *
 *  Created on: Oct 2, 2025
 *      Author: user
 */

#ifndef USER_INC_MUX_H_
#define USER_INC_MUX_H_

typedef enum { MUX_EXT=0, MUX_INT=1 } mux_sel_t;


void MUX_Init(void);
void MUX_Set(mux_sel_t sel);


/* Demo */
void MUX_Test(void);

#endif /* USER_INC_MUX_H_ */
