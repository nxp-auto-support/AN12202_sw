/*
 * sai.c
 *
 *  Created on: Jan 9, 2018
 *   Modified on: Mar 15, 2020
 *      Author: B50982, nxa3277
 */

#include "sai.h"
#include "dma.h"
#include "device_registers.h"

static void SAI_pins_init (void);
static void SAI_generate_bitClock (void);

void SAI_i2s_init(void)
{
    /* Turn SAI0 clock on */
    PCC->PCCn[PCC_SAI0_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Configure SAI0 pins */
    SAI_pins_init();

    /* Disable transmitter and receiver */
    SAI0->TCSR &= ~(SAI_TCSR_TE_MASK);
    SAI0->RCSR &= ~(SAI_RCSR_RE_MASK);

    /* Enable on debug mode and reset FIFO */
    SAI0->TCSR |= (SAI_TCSR_DBGE_MASK | SAI_TCSR_FR_MASK);
    SAI0->RCSR |= (SAI_RCSR_DBGE_MASK | SAI_RCSR_FR_MASK);

    /* Asynchronous mode for Transmitter, Receiver is configured in synchronous mode */
    SAI0->TCR2 &= ~SAI_TCR2_SYNC_MASK;
    SAI0->RCR2 |= SAI_RCR2_SYNC(1);

    /* Clock polarity:
     * 0 - Bit clock is active high with drive outputs on rising edge and sample inputs on falling edge.*/
    SAI0->TCR2 &= ~SAI_TCR2_BCP_MASK;
    SAI0->TCR2 |= SAI_TCR2_BCP(0);

    /* Frame Sync Early:
     * 1 - Frame sync asserts one bit before the first bit of the frame */
    /* Frame Sync Polarity:
     * 0 - Frame sync is active high. */
    /* Frame Sync Width:
     * Configures the length of the frame sync in number of bit clocks.
     * The value written must be one less than the number of bit clocks*/
    /* Data order:
     * 1 - MSB is transmitted first. */
    /* Frame Size: 2 words per frame */
    SAI0->TCR4 = SAI_TCR4_FSE(1) |
            SAI_TCR4_FSP(0) |
            SAI_TCR4_SYWD(SAI_BITS_PER_CHANNEL - 1) |
            SAI_TCR4_MF_MASK |
            (SAI_TCR4_FRSZ(1));

    /* Slot offset:
     * Configures the bit index for the first bit transmitted for each word in the frame */
    /* Slot size:
     * Configures the number of bits in each word and for first word independently */
    /* Slot count:
     * Configures the number of words in each frame.
     * The value written must be one less than the number of words in the frame*/
    SAI0->TCR5 = SAI_TCR5_FBT(SAI_BITS_PER_CHANNEL - 1) |
            SAI_TCR5_WNW(SAI_BITS_PER_CHANNEL - 1) |
            SAI_TCR5_W0W(SAI_BITS_PER_CHANNEL - 1);

    /* Generate FS and BCLK as SAI is configured as Master */
    SAI_generate_bitClock();

    /* Enable 2 slots/channels in the audio frame (bit 0 and bit 1 are set to zero) */
    SAI0->TMR = 0xFFFC;

    /* Disable all SAI0 data lines */
    SAI0->TCR3 &= ~SAI_TCR3_TCE_MASK;

    /* Configure watermark for transmitter at 6 entries */
    SAI0->TCR1 = SAI_TCR1_TFW(6);

    /* Enable SAI transmitter*/
    SAI0->TCSR |= (SAI_TCSR_TE_MASK);
}

void SAI_send_data (uint16_t * InitBuffer)
{

    uint8_t fifoIndex = 0;

    /*Fill FIFO*/
    for (fifoIndex = 0; fifoIndex < 8; fifoIndex++)
    {
        SAI0->TDR[0] = InitBuffer[fifoIndex];
    }

    SAI0->TCSR |= SAI_TCSR_FRDE_MASK;    /*Enable DMA operation*/

    DMA_enableRequest(DMA_CHANNEL_Tx);  /*Enable DMA request*/

    SAI0->TCR3 |= SAI_TCR3_TCE(1 << 0); /*Enable Transmit channel*/
}

static void SAI_pins_init (void)
{
    /* Initialize clock for PORTA */
    PCC->PCCn[PCC_PORTA_INDEX] = PCC_PCCn_CGC_MASK;

    /* PTA13 as SAI0_D0 */
    PORTA->PCR[13] = PORT_PCR_MUX(7);
    /* PTA13 as SAI0_BCLK */
    PORTA->PCR[12] = PORT_PCR_MUX(7);
    /* PTA13 as SAI0_SYNC */
    PORTA->PCR[11] = PORT_PCR_MUX(6);

    /* Configure PDDR register for SAI pins */
    PTA->PDDR |= ((1 << 11) | (1 << 12) | (1 << 13));
}

static void SAI_generate_bitClock (void)
{
    /* Bit clock is generated internally in Master mode.*/
    SAI0->TCR2 |= SAI_TCR2_BCD_MASK;
    /* Frame sync is generated internally in Master mode. */
    SAI0->TCR4 |= SAI_TCR4_FSD_MASK;

    /* Sample */
    /* BCLK = Bus clock / ((DIV + 1) * 2) */
    /* DIV = (SAI Clock / (128 * Sample Rate)) - 1*/
    SAI0->TCR2 &= ~(SAI_TCR2_MSEL_MASK | SAI_TCR2_DIV_MASK);
    SAI0->TCR2 |= (SAI_TCR2_MSEL(0) | SAI_TCR2_DIV(SAI_DIV_VALUE));
}

