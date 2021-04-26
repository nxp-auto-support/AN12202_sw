/*
 * dma.c
 *
 *  Created on: Apr 6, 2018
 *  Modified on: Mar 15, 2020
 *      Author: nxa12021, nxa32477
 */

#include "dma.h"
#include "config.h"
#include "dataTypes.h"
#include "nvic.h"

static void DMA_error (void);

void DMA_init (void)
{
    /* Turn DMAMUX clock on */
    PCC->PCCn[PCC_DMAMUX_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Turn DMA clock on */
    SIM->PLATCGC |= SIM_PLATCGC_CGCDMA_MASK;

    /* Configure DMA */
    DMA->CR |= DMA_CR_EMLM_MASK;	/*Enable MinorLoop Mapping*/

    /* Enable DMA error interrupt */
    setIRQCallback(DMA_Error_IRQn, DMA_error);
    EnableIRQ(DMA_Error_IRQn);
}

void DMA_setupChannel_SAI0_tx (uint8_t channel,
        uint32_t buffer0Address,
        uint32_t totalBufferSize,
        uint8_t bytesPerRequest,
        eDMA_srcDst_size size,
        vfcnPtr dmaCallback)
{
    uint8_t offsetInBytes;

    /* Valid channel ? */
    if (FEATURE_DMA_CHANNELS > channel)
    {
        /* Get the offset in bytes */
        switch (size)
        {
        case size_8bits:
            offsetInBytes = 1;
            break;
        case size_16bits:
            offsetInBytes = 2;
            break;
        case size_32bits:
            offsetInBytes = 4;
            break;
        case size_16ByteBurst:
            offsetInBytes = 16;
            break;
        case size_32ByteBurst:
            offsetInBytes = 32;
            break;
        default:
            break;
        }
        /* Enable DMA channel */
        DMAMUX->CHCFG[channel] = DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(DMA_SOURCE_SAI0_TX);

        /* Set size for destination and source */
        DMA->TCD[channel].ATTR = DMA_TCD_ATTR_DSIZE(size) |
                DMA_TCD_ATTR_SSIZE(size);

        /* Set the source address, offset and last adjustment */
        DMA->TCD[channel].SADDR = (int32_t)buffer0Address;
        DMA->TCD[channel].SOFF = (int32_t)totalBufferSize;/*Offset to the next buffer*/
        DMA->TCD[channel].SLAST = (uint32_t)(-(totalBufferSize*3)+offsetInBytes);


        /*Clear NBYTES and Minor Loop Offset bits*/
        DMA->TCD[channel].NBYTES.MLOFFYES = 0;

        /*How many bytes will be requested per DMA's trigger?
         * Minor loop offset of - 2 * totalbuffersize + Word size*/
        DMA->TCD[channel].NBYTES.MLOFFYES |= DMA_TCD_NBYTES_MLOFFYES_NBYTES(bytesPerRequest)
                                            | DMA_TCD_NBYTES_MLOFFYES_MLOFF(offsetInBytes - (totalBufferSize << 1))
                                            | DMA_TCD_NBYTES_MLOFFYES_SMLOE_MASK;

        /* Set destination address, offset and last adjustment/Scatter-gatter address */
        DMA->TCD[channel].DADDR       = (int32_t)&(SAI0->TDR[0]);
        DMA->TCD[channel].DOFF        = 0;           /* No increment, always point to same direction */
        DMA->TCD[channel].DLASTSGA    = 0;           /* No increment, always point to same direction */


        /* How many iterations are needed to send all data */
        DMA->TCD[channel].CITER.ELINKNO = (totalBufferSize / bytesPerRequest) * 2; /* 2 times, left and right*/
        DMA->TCD[channel].BITER.ELINKNO = (totalBufferSize / bytesPerRequest) * 2; /* 2 times, left and right*/

        /* Interrupt when channel is completed, disable channel request after completion */
        DMA->TCD[channel].CSR = DMA_TCD_CSR_INTMAJOR_MASK | DMA_TCD_CSR_DREQ_MASK;

        /* Enable DMA error for selected channel*/
        DMA->EEI |= (1 << channel);

        /* Interrupt when channel is completed */
        /* Configure IRQ */
        setIRQCallback(DMA0_IRQn + channel, dmaCallback);
        EnableIRQ(DMA0_IRQn + channel);

        /* Enable request */
        DMA->ERQ |= (1 << channel);
    }
}


void DMA_setupChannel_SAI1_rx (uint8_t channel,
        uint32_t buffer0Address,
        uint32_t totalBufferSize,
        uint8_t bytesPerRequest,
        eDMA_srcDst_size size,
        vfcnPtr dmaCallback)
{
    uint8_t offsetInBytes;

    /* Valid channel ? */
    if (FEATURE_DMA_CHANNELS > channel)
    {
        /* Get the offset in bytes */
        switch (size)
        {
        case size_8bits:
            offsetInBytes = 1;
            break;
        case size_16bits:
            offsetInBytes = 2;
            break;
        case size_32bits:
            offsetInBytes = 4;
            break;
        case size_16ByteBurst:
            offsetInBytes = 16;
            break;
        case size_32ByteBurst:
            offsetInBytes = 32;
            break;
        default:
            break;
        }

        /* Enable DMA channel */
        DMAMUX->CHCFG[channel] = DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(DMA_SOURCE_SAI1_RX);

        /* Set size for destination and source */
        DMA->TCD[channel].ATTR =  DMA_TCD_ATTR_DSIZE(size) |
                DMA_TCD_ATTR_SSIZE(size);

        /* Set the source address, offset and last adjustment */
        DMA->TCD[channel].SADDR = (int32_t)&(SAI1->RDR[0]);
        DMA->TCD[channel].SOFF = 0;
        DMA->TCD[channel].SLAST = 0;

        /* Set destination address, offset and last adjustment/Scatter-gatter address */
        DMA->TCD[channel].DADDR       = (int32_t)buffer0Address;
        DMA->TCD[channel].DOFF        = (int32_t)totalBufferSize;  /*Offset to the next buffer*/
        DMA->TCD[channel].DLASTSGA    = (uint32_t)(-(totalBufferSize*3)+offsetInBytes);

        /* How many bytes will be requested per DMA's trigger? */
        DMA->TCD[channel].NBYTES.MLOFFYES = 0;
        DMA->TCD[channel].NBYTES.MLOFFYES |= DMA_TCD_NBYTES_MLOFFYES_NBYTES(bytesPerRequest)
                                            | DMA_TCD_NBYTES_MLOFFYES_MLOFF((offsetInBytes-(totalBufferSize<<1)))
                                            | DMA_TCD_NBYTES_MLOFFYES_DMLOE_MASK;

        /* How many iterations are needed to send all data */
        DMA->TCD[channel].CITER.ELINKNO = (totalBufferSize / bytesPerRequest) * 2; /* 2 times, left and right*/
        DMA->TCD[channel].BITER.ELINKNO = (totalBufferSize / bytesPerRequest) * 2; /* 2 times, left and right*/

        /* Interrupt when channel is completed*/
        DMA->TCD[channel].CSR = DMA_TCD_CSR_INTMAJOR_MASK | DMA_TCD_CSR_DREQ_MASK;

        /* Enable DMA error for selected channel*/
        DMA->EEI |= (1 << channel);

        /* Configure IRQ */
        setIRQCallback(DMA0_IRQn + channel, dmaCallback);
        EnableIRQ(DMA0_IRQn + channel);

        /* Enable request */
        DMA->ERQ |= (1 << channel);
    }
}


inline void DMA_enableRequest (int8_t channel)
{
    /* Enable request */
    DMA->EARS |= (1 << channel);
    DMA->ERQ |= (1 << channel);
}


void DMA_clearCompleteFlag (uint8_t channel)
{
    DMA->INT |= 1 << channel;
}

static void DMA_error (void)
{
    volatile uint32_t dmaError __attribute__ ((unused)) = DMA->ERR;
    /* Clear DMA errors */
    DMA->ERR |= 0xFFFF;
    while (1)
    {

    }
}
