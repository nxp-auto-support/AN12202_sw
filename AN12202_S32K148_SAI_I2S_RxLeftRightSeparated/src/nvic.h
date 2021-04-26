/*
 * nvic.h
 *
 *  Created on: Mar 15, 2017
 *      Author: B50982
 */

#ifndef NVIC_H_
#define NVIC_H_

/* Only include these header files */
#include "device_registers.h"
#include "dataTypes.h"

#define MAX_IRQ_NUM                     FTM7_Ovf_Reload_IRQn

/**************************************************************************//*!
 *
 * @name    EnableIRQ
 *
 * @brief   Enable IRQ for desired vector
 *
 * @param   irq: IRQ to be enabled (starting from 0 and not considering core vectors)
 *
 * @return  eNoError: Configuration was done without problems.
 *          eInvalidParameter: function received invalid parameters.
 *****************************************************************************/
int8_t EnableIRQ (IRQn_Type irq);

/**************************************************************************//*!
 *
 * @name    DisableIRQ
 *
 * @brief   Disable IRQ for desired vector
 *
 * @param   irq: IRQ to be disabled (starting from 0 and not considering core vectors)
 *
 * @return  eNoError: Configuration was done without problems.
 *          eInvalidParameter: function received invalid parameters.
 *****************************************************************************/
int8_t DisableIRQ (IRQn_Type irq);

/**************************************************************************//*!
 *
 * @name    SetIRQPriority
 *
 * @brief   Set priority for desired IRQ
 *
 * @param   irq: IRQ to modify its priority
 *          priority: priority for the IRQ, 0 highest and 15 the lowest.
 *
 * @return  eNoError: Configuration was done without problems.
 *          eInvalidParameter: function received invalid parameters.
 *****************************************************************************/
int8_t SetIRQPriority (IRQn_Type irq, uint8_t priority);

/**************************************************************************//*!
 *
 * @name    setIRQCallback
 *
 * @brief   Set callback for IRQ
 *
 * @param   irq: Desired IRQ
 *          callback: Pointer to function for desired callback. It is a
 *          void-void function.
 *
 * @return  eNoError: Configuration was done without problems.
 *          eNullPointer: If pointer to function is NULL.
 *          eInvalidParameter: function received invalid parameters.
 *****************************************************************************/
int8_t setIRQCallback (IRQn_Type irq, vfcnPtr callback);

#endif /* NVIC_H_ */
