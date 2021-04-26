/*
 * main implementation: use this 'C' sample to create your own application
 *
 */


#include "S32K148.h" /* include peripheral declarations S32K148 */
#include "clocks.h"
#include "sai.h"
#include "dma.h"
#include "config.h"

static void systemInit (void);

uint16_t Buffers[2][BUFFER_SIZE] = {
    {0x1234, 0x1234, 0x1234, 0x1234, 0x1234, 0x1234, 0x1234, 0x1234},
    {0x4321, 0x4321, 0x4321, 0x4321, 0x4321, 0x4321, 0x4321, 0x4321}
};

uint8_t CurrentBuffer;
uint8_t FreeBuffer;
uint8_t BufferProcessing = 8;
uint16_t *ProcessingBuffferPtr = &Buffers[0][0];

void DmaCallback(void)
{
    /* Clear DMA flag */
    DMA_clearCompleteFlag(DMA_CHANNEL_Tx);
    /* Get buffer for current TCD */
    CurrentBuffer = DMA_getCurrentDescriptor(DMA_CHANNEL_Tx);
    /*Get which buffer is free*/
    FreeBuffer = (~CurrentBuffer) & 0x01;
#if (1 == ENABLE_TCD_PIN)
    PTA->PDOR = CurrentBuffer << 25;
#endif
    /* Buffer processing is started on first element */
    BufferProcessing = 0;
    /* Processing buffer ptr set to free Buffer */
    ProcessingBuffferPtr = Buffers[FreeBuffer];
}

#if (1 == ENABLE_TCD_PIN)
void DebugTcdPinInit(void)
{
    /*Enable Port A Clock*/
    PCC->PCCn[PCC_PORTA_INDEX] = PCC_PCCn_CGC_MASK;
    /*Configure as GPIO*/
    PORTA->PCR[CURRENT_TCD_PIN] = PORT_PCR_MUX(1);
    /*Configure as Output*/
    PTA->PDDR |= (1<<CURRENT_TCD_PIN);
}
#endif

int main(void)
{
    /* Initialize system */
    systemInit();

    /* Clock settings */
    RUN_mode_80MHz();

#if (1 == ENABLE_TCD_PIN)
    /*Init TCD debug pin PTA25)*/
    DebugTcdPinInit();
#endif

    /* Initialize SAI module */
    SAI_i2s_init();

    /* Set DMA up */
    DMA_init();

    /* Prepare TCD for SAI0_Tx operation */
    DMA_setupChannel_SAI0_txPingPongBuffers(DMA_CHANNEL_Tx,
            (uint32_t)Buffers[0],
            (uint32_t)Buffers[1],
            BUFFER_SIZE * WORD_SIZE,    /*Buffer size in bytes*/
            WORD_SIZE << 1,             /*Send two words per request*/
            size_16bits,
            &DmaCallback
    );

    /* Get current descriptor index */
    CurrentBuffer = DMA_getCurrentDescriptor(DMA_CHANNEL_Tx);

    /* Start sending data on SAI */
    SAI_send_data((uint16_t *)&Buffers[1]);

    while (1)
    {
        while(BufferProcessing < BUFFER_SIZE)
        {
            ProcessingBuffferPtr[BufferProcessing++]++;
        }
    }
    return 0;
}

static void systemInit (void)
{
    /* Disable MPU */
    MPU->CESR &= ~(MPU_CESR_VLD_MASK);

    /* Round robin priority for Crossbar */
    MCM->CPCR |= MCM_CPCR_CBRR_MASK;
    /* Cache Enabled (Invalidate Way 1 and Way 0) */
    LMEM->PCCCR = LMEM_PCCCR_GO_MASK |
            LMEM_PCCCR_INVW1_MASK |
            LMEM_PCCCR_INVW0_MASK |
            LMEM_PCCCR_ENCACHE_MASK;

    /* Speculation for instruction enabled and speculation for data disabled (P-Flash) */
    MSCM->OCMDR[0] = MSCM_OCMDR_OCM1(2);
    /* Speculation for instruction enabled and speculation for data disabled (D-Flash) */
    MSCM->OCMDR[1] = MSCM_OCMDR_OCM1(2);
}
