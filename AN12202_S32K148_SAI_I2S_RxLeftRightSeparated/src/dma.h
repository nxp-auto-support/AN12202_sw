/*
 * dma.h
 *
 *  Created on: Apr 6, 2018
 *   Modified on: Mar 15, 2020
 *      Author: nxa12021, nxa32477
 */

#ifndef DMA_H_
#define DMA_H_

#include "device_registers.h"
#include "dataTypes.h"

typedef enum {
    size_8bits  = 0,
    size_16bits,
    size_32bits,
    size_16ByteBurst = 4,
    size_32ByteBurst
}eDMA_srcDst_size;

void DMA_init (void);

void DMA_setupChannel_SAI0_tx (uint8_t channel,
        uint32_t buffer0Address,
        uint32_t totalBufferSize,
        uint8_t bytesPerRequest,
        eDMA_srcDst_size size,
        vfcnPtr dmaCallback);

void DMA_setupChannel_SAI1_rx (uint8_t channel,
        uint32_t buffer0Address,
        uint32_t totalBufferSize,
        uint8_t bytesPerRequest,
        eDMA_srcDst_size size,
        vfcnPtr dmaCallback);

extern inline void DMA_enableRequest (int8_t channel);

void DMA_clearCompleteFlag (uint8_t channel);

#endif /* DMA_H_ */
