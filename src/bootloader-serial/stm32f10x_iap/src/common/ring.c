// ring.c
//

#include "common/ring.h"
#include "diag/Trace.h"


/////////////////////////////////////////////////////////////////////////////////////////
//

void ring_init(RING_BUFFER * rb)
{
	rb->front = rb->rear = 0;
}

int32_t ring_getBufferSize(RING_BUFFER * rb)
{
	(void)(rb);

	//
	return MAX_RINGBUF_DATASIZE;
}

int32_t ring_getDataSize(RING_BUFFER * rb)
{
	//
	return (rb->front - rb->rear) & (MAX_RINGBUF_DATASIZE - 1);
}

int32_t ring_isEmpty(RING_BUFFER * rb)
{
	return (rb->front == rb->rear) ? 1 : 0;
}

int32_t ring_isFull(RING_BUFFER * rb)
{
	return ((rb->front + 1) & (MAX_RINGBUF_DATASIZE - 1)) == rb->rear ? 1 : 0;
}

int32_t ring_push(RING_BUFFER * rb, int32_t  c)
{
	if (ring_isFull(rb))
		return -1;

	rb->data[rb->front] = c;
	rb->front = (rb->front + 1) & (MAX_RINGBUF_DATASIZE -1);

	return 0;
}

int32_t ring_pop(RING_BUFFER * rb)
{
	if (ring_isEmpty(rb))
		return -1;

	int32_t ch = rb->data[rb->rear];
	rb->rear = (rb->rear + 1) & (MAX_RINGBUF_DATASIZE -1);

	return ch;
}
