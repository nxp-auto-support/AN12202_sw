/*
 * sai.h
 *
 *  Created on: Jan 9, 2018
 *   Modified on: Mar 15, 2020
 *      Author: B50982, nxa3277
 */

#ifndef SAI_H_
#define SAI_H_

#include "device_registers.h"
#include "config.h"

void SAI_i2s_init(void);

void SAI_send_data (uint16_t * InitBuffer);

#endif /* SAI_H_ */
