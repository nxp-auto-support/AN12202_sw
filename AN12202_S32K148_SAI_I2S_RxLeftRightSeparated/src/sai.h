/*
 * sai.h
 *
 *  Created on: Jan 9, 2018
 *   Modified on: Mar 15, 2020
 *      Author: B50982, nxa32477
 */

#ifndef SAI_H_
#define SAI_H_

#include "device_registers.h"
#include "config.h"

void SAI_i2s_init(void);

void SAI1_i2s_rx_slave_init(void);

void SAI1_receive_data(uint32_t* txBuffer);

void SAI_i2s_fillTxBuffer (void);

void SAI0_Enable_Tx(void);

void SAI1_Enable_Rx(void);

#endif /* SAI_H_ */
