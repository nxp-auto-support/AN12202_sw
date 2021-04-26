/*
 * config.h
 *
 *  Created on: Apr 6, 2018
 *      Author: nxa12021
 */

#ifndef CONFIG_H_
#define CONFIG_H_

/* DMA channels */
#define DMA_CHANNEL_Tx          (0)
#define DMA_CHANNEL_Rx          (1)

/* DMA request options */
#define DMA_SOURCE_SAI0_TX      (58)
#define DMA_SOURCE_SAI1_RX      (12)

/* Definitions for words per frame
 * and total words in the transaction */
#define BUFFER_SIZE             (8)
#define WORD_SIZE               (2)

/* Bits per channel and SAI Divider */
#define SAI_BITS_PER_CHANNEL    (16)
#define SAI_DIV_VALUE           (12)

/* GPIO configuration */
#define ENABLE_TCD_PIN          (1)
#define CURRENT_TCD_PIN         (25)


#endif /* CONFIG_H_ */
