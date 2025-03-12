/*
 * fifo.h
 *
 *  Created on: Mar 12, 2025
 *      Author: User123
 */

#ifndef INC_FIFO_H_
#define INC_FIFO_H_

/* Exported Includes ----------------------------------------------------------*/
#include <stdint.h>

/* Exported typedef -----------------------------------------------------------*/
typedef struct
{
	int16_t *buffer;	// Wr/Rd PCM FIFO buffer in half-words
	uint16_t capacity;
	uint8_t wr_idx;		// FIFO write pointer
	uint8_t rd_idx;		// FIFO read pointer
	uint16_t numel;		// FIFO element count
} Fifo_t;

/* Exported define ------------------------------------------------------------*/

/* Exported macro -------------------------------------------------------------*/

/* Exported variables ---------------------------------------------------------*/

/* Exported function prototypes -----------------------------------------------*/
void FIFO_Init(Fifo_t *pFifo, int16_t *pBuffer, uint16_t len);
int FIFO_Write(Fifo_t *pFifo, int16_t din);
int FIFO_Read(Fifo_t *pFifo, int16_t *dout);
int FIFO_WriteBlock(Fifo_t *pFifo, const int16_t *buffer, uint16_t len);
int FIFO_ReadBlock(Fifo_t *pFifo, int16_t *buffer, uint16_t len);

#endif /* INC_FIFO_H_ */
