/*
 * main implementation: use this 'C' sample to create your own application
 *
 */


#include "S32K148.h" /* include peripheral declarations S32K148 */
#include "sai.h"
#include "dma.h"
#include "clocks.h"

static void DmaCallbackTx(void);

uint16_t TxBuffer[2][BUFFER_SIZE] __attribute__ ((aligned(32)));

/* Reception buffer allocating space for both right and left channels plus 4 additional
 * words implemented by transmitter initialization FIFO procedure */
uint16_t RxBuffer[2][BUFFER_SIZE + 4] __attribute__ ((aligned(32))) = {{0}};

/* Callback for Tx Operation */
void DmaCallbackTx(void)
{
    DMA_clearCompleteFlag(DMA_CHANNEL_Tx);
}

/* Callback for Rx Operation */
void DmaCallbackRx(void)
{
    DMA_clearCompleteFlag(DMA_CHANNEL_Rx);
}

int main(void)
{
    /* Configure MCU to work in RUN mode at 80 MHz (SYS_CLK) */
    RUN_mode_80MHz();

    uint32_t i;
    for (i = 0; i<BUFFER_SIZE; i++)
    {
    	TxBuffer[0][i] = i;
		TxBuffer[1][i] = 0x1000 + i;
    }

    /* Initialize DMA module, enable minor loop offset */
    DMA_init();

    /* Configure DMA channel for SAI0 (transmitter) */
    DMA_setupChannel_SAI0_tx(DMA_CHANNEL_Tx,
            (uint32_t)TxBuffer[0],
            BUFFER_SIZE * WORD_SIZE,    /* Buffer size in bytes */
            WORD_SIZE<<1,               /* Send two words per request */
            size_16bits,
            &DmaCallbackTx);

    /* Configure DMA channel for SAI1 (receiver) */
    DMA_setupChannel_SAI1_rx(DMA_CHANNEL_Rx,
            (uint32_t)RxBuffer[0],
            (BUFFER_SIZE+4) * WORD_SIZE,    /*Buffer size in bytes*/
            WORD_SIZE<<1,                   /*Send two words per request*/
            size_16bits,
            &DmaCallbackRx);

    /* Configure SAI1 as receiver */
    SAI1_i2s_rx_slave_init();

    /* Enable chanel and DMA requests */
    SAI1_Enable_Rx();

    /* Configure SAI0 as transmitter */
    SAI_i2s_init();

    /* Fill TX buffer*/
    SAI_i2s_fillTxBuffer();

    /* Enable chanel and DMA requests */
    SAI0_Enable_Tx();

    for(;;)
    {

    }
    return 0;
}
