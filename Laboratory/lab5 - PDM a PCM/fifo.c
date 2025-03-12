/*
 * fifo.c
 *
 *  Created on: Mar 12, 2025
 *      Author: User123
 */

/* Private Includes ----------------------------------------------------------*/
#include "fifo.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported function reference -----------------------------------------------*/
void FIFO_Init(Fifo_t *pFifo, int16_t *pBuffer, uint16_t len)
{
	pFifo->buffer = pBuffer;
	pFifo->capacity = len;
	pFifo->numel = 0;
	pFifo->rd_idx = 0;
	pFifo->wr_idx = 0;
}

int FIFO_Write(Fifo_t *pFifo, int16_t din)
{
	if(pFifo->numel == pFifo->capacity) return -1;

	pFifo->buffer[pFifo->wr_idx++] = din;
	if(pFifo->wr_idx >= pFifo->capacity) pFifo->wr_idx = 0;

	pFifo->numel++;

	return 0;
}

int FIFO_Read(Fifo_t *pFifo, int16_t *dout)
{
	if(!pFifo->numel) return -1;

	*dout = pFifo->buffer[pFifo->rd_idx++];
	if(pFifo->rd_idx >= pFifo->capacity) pFifo->rd_idx = 0;

	pFifo->numel--;

	return 0;
}

int FIFO_WriteBlock(Fifo_t *pFifo, const int16_t *buffer, uint16_t len)
{
	if(pFifo->numel == pFifo->capacity) return -1;

	int uAudioWordsWritten = 0;
	while(len-- && pFifo->numel++ < pFifo->capacity)
	{
		pFifo->buffer[pFifo->wr_idx++] = *buffer++;
		if(pFifo->wr_idx >= pFifo->capacity) pFifo->wr_idx = 0;
		uAudioWordsWritten++;
	}

	return uAudioWordsWritten;
}

int FIFO_ReadBlock(Fifo_t *pFifo, int16_t *buffer, uint16_t len)
{
	if(!pFifo->numel) return -1;

	int uAudioWordsRead = 0;
	while(len-- && pFifo->numel--)
	{
		*buffer++ = pFifo->buffer[pFifo->rd_idx++];
		if(pFifo->rd_idx >= pFifo->capacity) pFifo->rd_idx = 0;
		uAudioWordsRead++;
	}

	return uAudioWordsRead;
}
