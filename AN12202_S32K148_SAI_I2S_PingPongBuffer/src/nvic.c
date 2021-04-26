/*
 * nvic.c
 *
 *  Created on: Mar 15, 2017
 *      Author: B50982
 */

#include "nvic.h"
#include "dataTypes.h"

/* Allocate vector table in RAM, all IRQ plus 16 core exceptions */
uint32_t __VECTOR_RAM[((uint32_t)(MAX_IRQ_NUM)) + 16U + 1U];

int8_t EnableIRQ (IRQn_Type irq)
{
    int8_t ret = 0;
    if ((irq <= MAX_IRQ_NUM) &&
            (irq >= 0))
    {
        /* Clear pending interrupt */
        S32_NVIC->ICPR[(irq >> 5)] |= 1 << (irq & 0x1F);
        /* Enable interrupt */
        S32_NVIC->ISER[(irq >> 5)] |= 1 << (irq & 0x1F);
    }
    else
    {
        /* Invalid IRQ */
        ret = -1;
    }
    return ret;
}

int8_t DisableIRQ (IRQn_Type irq)
{
    int8_t ret = 0;
    if ((irq <= MAX_IRQ_NUM) &&
            (irq >= 0))
    {
        /* Disable interrupt */
        S32_NVIC->ICER[(irq >> 5)] |= 1 << (irq & 0x1F);
    }
    else
    {
        /* Invalid IRQ */
        ret = -1;
    }
    return ret;
}

int8_t SetIRQPriority (IRQn_Type irq, uint8_t priority)
{
    int8_t ret = 0;
    if ((irq <= MAX_IRQ_NUM) &&
            (irq >= 0))
    {
        /* Set priority (4-most significant bits in the 8-bit register */
        S32_NVIC->IP[(irq >> 2)] = (priority & 0xF) << 4;
    }
    else
    {
        /* Invalid IRQ */
        ret = -1;
    }
    return ret;
}

int8_t setIRQCallback (IRQn_Type irq, vfcnPtr callback)
{
    int8_t ret = 0;
    /* Validate pointer */
    if (NULL != callback)
    {
        if ((irq <= MAX_IRQ_NUM) &&
                (irq >= 0))
        {
            /* Set handler into vector table */
            __VECTOR_RAM[((int32_t)irq) + 16] = (uint32_t)callback;
        }
        else
        {
            /* Invalid IRQ */
            ret = -1;
        }
    }
    else
    {
        /* NULL callback */
        ret = -2;
    }
    return ret;
}
