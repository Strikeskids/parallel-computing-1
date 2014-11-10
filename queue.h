#include <stdlib.h>

#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_MAKE(QUEUE_TYPE, QUEUE_NAME) \
\
typedef struct {\
	QUEUE_TYPE *items;\
	unsigned long start;\
	unsigned long end;\
	unsigned long capacity;\
} QUEUE_NAME;\
\
void queue_init(QUEUE_NAME *q, unsigned long capacity) {\
	q->capacity = capacity+1;\
	q->items = (QUEUE_TYPE *) malloc(sizeof(QUEUE_TYPE) * q->capacity);\
	q->start = 0;\
	q->end = 0;\
}\
\
QUEUE_NAME *queue_create(unsigned long capacity) {\
	QUEUE_NAME *q = (QUEUE_NAME *) malloc(sizeof(QUEUE_NAME));\
	queue_init(q, capacity);\
	return q;\
}\
\
unsigned long queue_length(QUEUE_NAME *q) {\
	return q->start > q->end ? q->capacity - q->start + q->end : q->end - q->start;\
}\
\
void queue_extend(QUEUE_NAME *q, unsigned long newLength) {\
	if (newLength <= q->capacity) return;\
	q->items = (QUEUE_TYPE *) realloc(q->items, newLength * sizeof(QUEUE_TYPE));\
	if (q->start > q->end) {\
		if (q->end <= newLength - q->capacity) {\
			memcpy(q->items + q->capacity, q->items, q->end * sizeof(QUEUE_TYPE));\
			q->end += q->capacity;\
		} else {\
			unsigned long numToMove = newLength - q->capacity;\
			memcpy(q->items + q->capacity, q->items, numToMove * sizeof(QUEUE_TYPE));\
			q->end -= numToMove;\
			memmove(q->items, q->items + numToMove, q->end * sizeof(QUEUE_TYPE));\
		}\
	}\
	q->capacity = newLength;\
	q->end %= q->capacity;\
}\
\
void queue_push(QUEUE_NAME *q, QUEUE_TYPE item) {\
	if ((q->end + 1) % q->capacity == q->start) {\
		queue_extend(q, q->capacity * 2 - 1);\
	}\
	q->items[q->end++] = item;\
	q->end %= q->capacity;\
}\
\
QUEUE_TYPE queue_pop(QUEUE_NAME *q) {\
	QUEUE_TYPE ans = q->items[q->start++];\
	q->start %= q->capacity;\
	return ans;\
}\
\
void queue_clear(QUEUE_NAME *q) {\
	q->start = 0;\
	q->end = 0;\
}\
\
QUEUE_TYPE queue_get(QUEUE_NAME *q, long n) {\
	return q->items[(q->start + n) % q->capacity];\
}\

#endif
