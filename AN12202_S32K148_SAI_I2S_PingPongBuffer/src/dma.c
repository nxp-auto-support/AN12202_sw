/*
 * dma.c
 *
 *  Created on: Apr 6, 2018
 *   Modified on: Mar 15, 2020
 *      Author: nxa12021, nxa32477
 */

#include "dma.h"
#include "config.h"
#include "dataTypes.h"
#include "nvic.h"

static void DMA_error (void);

typedef struct {
    uint32_t SADDR;
    uint16_t SOFF;
    uint16_t ATTR;
    uint32_t NBYTES;
    uint32_t SLAST;
    uint32_t DADDR;
    uint16_t DOFF;
    uint16_t CITER;
    uint32_t DLAST_SGA;
    uint16_t CSR;
    uint16_t BITER;
}TCD_t;

/* 2 TCDs: one for each ping pong buffer */
static TCD_t TCDs[2] __attribute__((aligned (32)));

void DMA_init (void)
{
    /* Turn DMAMUX clock on */
    PCC->PCCn[PCC_DMAMUX_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Turn DMA clock on */
    SIM->PLATCGC |= SIM_PLATCGC_CGCDMA_MASK;
    /* Configure DMA */
    DMA->CR = 0;

    /* Enable DMA error interrupt */
    setIRQCallback(DMA_Error_IRQn, DMA_error);
    EnableIRQ(DMA_Error_IRQn);
}

void DMA_setupChannel_SAI0_txPingPongBuffers (uint8_t channel,
        uint32_t buffer0Address,
        uint32_t buffer1Address,
        uint32_t totalBufferSize,
        uint8_t bytesPerRequest,
        eDMA_srcDst_size size,
        vfcnPtr dmaCallback)
{
    uint32_t offsetInBytes = 0;

    /* Valid channel ? */
    if (FEATURE_DMA_CHANNELS > channel)
    {
        /* Enable DMA channel */
        DMAMUX->CHCFG[channel] = DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(DMA_SOURCE_SAI0_TX);

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

        /* Set size for destination and source */
        TCDs[0].ATTR =  DMA_TCD_ATTR_DSIZE(size) |
                DMA_TCD_ATTR_SSIZE(size);
        TCDs[1].ATTR =  DMA_TCD_ATTR_DSIZE(size) |
                DMA_TCD_ATTR_SSIZE(size);

        /* Set the source address, offset and last adjustment */
        TCDs[0].SADDR = (int32_t)buffer0Address;
        TCDs[0].SOFF = (int32_t)offsetInBytes;
        TCDs[0].SLAST = 0;
        TCDs[1].SADDR = (int32_t)buffer1Address;
        TCDs[1].SOFF = (int32_t)offsetInBytes;
        TCDs[1].SLAST = 0;

        /* How many bytes will be requested per DMA's trigger? */
        TCDs[0].NBYTES = bytesPerRequest;
        TCDs[1].NBYTES = bytesPerRequest;

        /* Set destination address, offset and last adjustment/Scatter-gatter address */
        TCDs[0].DADDR       = (int32_t)&(SAI0->TDR[0]);
        TCDs[0].DOFF        = 0;           /* No increment, always point to same direction */
        TCDs[0].DLAST_SGA   = (int32_t)&TCDs[1];    /* Address for the next TCD that will be loaded once the current TCD is completed */
        /* Set destination address, offset and last adjustment/Scatter-gatter address */
        TCDs[1].DADDR       = (int32_t)&(SAI0->TDR[0]);
        TCDs[1].DOFF        = 0;           /* No increment, always point to same direction */
        TCDs[1].DLAST_SGA   = (int32_t)&TCDs[0];    /* Address for the next TCD that will be loaded once the current TCD is completed */

        /* How many iterations are needed to send all data */
        TCDs[0].CITER = (totalBufferSize / bytesPerRequest);
        TCDs[0].BITER = TCDs[0].CITER;
        TCDs[1].CITER = TCDs[0].CITER;
        TCDs[1].BITER = TCDs[0].BITER;

        /* Interrupt when channel is completed and enable scatter/gather feature */
        TCDs[0].CSR = DMA_TCD_CSR_INTMAJOR_MASK |
                DMA_TCD_CSR_ESG_MASK;
        /* Interrupt when channel is completed and enable scatter/gather feature */
        TCDs[1].CSR = DMA_TCD_CSR_INTMAJOR_MASK |
                DMA_TCD_CSR_ESG_MASK;

        /* Load TCD[0] in system */
        DMA->TCD[channel].ATTR  = TCDs[0].ATTR;
        DMA->TCD[channel].SADDR = TCDs[0].SADDR;
        DMA->TCD[channel].SOFF  = TCDs[0].SOFF;
        DMA->TCD[channel].SLAST = TCDs[0].SLAST;
        /* How many bytes will be requested per DMA's trigger? */
        DMA->TCD[channel].NBYTES.MLNO = TCDs[0].NBYTES;

        DMA->TCD[channel].DADDR    = TCDs[0].DADDR;
        DMA->TCD[channel].DOFF     = TCDs[0].DOFF; /* No increment, always point to same direction */
        DMA->TCD[channel].DLASTSGA = TCDs[0].DLAST_SGA;

        DMA->TCD[channel].CITER.ELINKNO = TCDs[0].CITER;
        DMA->TCD[channel].BITER.ELINKNO = TCDs[0].BITER;

        /* Interrupt when channel is completed */
        DMA->TCD[channel].CSR = TCDs[0].CSR;

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

uint8_t DMA_getCurrentDescriptor (uint8_t channel)
{
    return (TCDs[0].SADDR == DMA->TCD[channel].SADDR ? 0 : 1);
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
