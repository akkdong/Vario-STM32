// ring.h
//

#ifndef __RING_H__
#define __RING_H__

#include "stm32f10x.h"


/////////////////////////////////////////////////////////////////////////////////////////
//

#define MAX_RINGBUF_DATASIZE		64



/////////////////////////////////////////////////////////////////////////////////////////
//

typedef struct
{
	uint8_t		data[MAX_RINGBUF_DATASIZE];

	uint16_t	front;
	uint16_t	rear;

} RING_BUFFER;



/////////////////////////////////////////////////////////////////////////////////////////
//

void 		ring_init(RING_BUFFER * rb);

int32_t 	ring_getBufferSize(RING_BUFFER * rb);
int32_t 	ring_getDataSize(RING_BUFFER * rb);

int32_t 	ring_isEmpty(RING_BUFFER * rb);
int32_t 	ring_isFull(RING_BUFFER * rb);

int32_t 	ring_push(RING_BUFFER * rb, int32_t c);
int32_t 	ring_pop(RING_BUFFER * rb);


#endif // __RING_H__
