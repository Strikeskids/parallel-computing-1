#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EMPTY '-'
#define TREE 'T'
#define FIRE '*'

typedef struct {
	long *items;
	long start;
	long end;
	long length;
} Queue;

const int forestWidth = 30;
const int forestHeight = 30;

void swap(void **a, void **b) {
	void *tmp = *a;
	*a = *b;
	*b = tmp;
}

void printTrees(char *trees, long width, long height) {
	sleep(1);
	printf("\x1B[1;1H");

	long total = width * height;
	long i;
	for (i=0;i<total;) {
		printf("%c", trees[i]);
		if (++i % width == 0) printf("\n");
	}
}

Queue *queue_init(long length) {
	Queue *q = (Queue *) malloc(sizeof(Queue));
	q->length = length;
	q->items = (long *) malloc(sizeof(long) * q->length);
	q->start = 0;
	q->end = 0;
}

long queue_length(Queue *q) {
	return q->start > q->end ? q->length - q->start + q->end : q->end - q->start;
}

void queue_extend(Queue *q, long newLength) {
	if (newLength <= q->length) return;
	q->items = (long *) realloc(q->items, newLength * sizeof(long));
	if (q->start > q->end) {
		if (q->end < newLength - q->length) {
			memcpy(q->items + q->length, q->items, q->end);
			q->end += q->length;
		} else {
			long numToMove = newLength - q->length;
			memcpy(q->items + q->length, q->items, numToMove);
			memmove(q->items, q->items + numToMove, q->end - numToMove);
			q->end -= numToMove;
		}
	}
	q->length = newLength;
}

void queue_push(Queue *q, long item) {
	if ((q->end + 1)%q->length == q->start) {
		queue_extend(q, q->length * 2);
	}
	q->items[q->end++] = item;
	q->end %= q->length;
}

long queue_pop(Queue *q) {
	long ans = q->items[q->start++];
	q->start %= q->length;
	return ans;
}

long queue_clear(Queue *q) {
	q->start = 0;
	q->end = 0;
}

int main(int argc, char ** argv) {
	float probability, randPoint;
	probability = 1;
	randPoint = probability + RAND_MAX * probability;

	char *trees;

	long i, len;

	trees = (char *) malloc(forestWidth * forestHeight * sizeof(char));
	memset(trees, EMPTY, forestWidth * forestHeight);

	for (i=0,len=forestWidth*forestHeight;i<len;++i) {
		if (rand() < randPoint) {
			trees[i] = TREE;
		}
	}

	Queue *current, *next;
	current = queue_init(forestHeight);
	next = queue_init(forestHeight);
	
	printf("\x1B[2J");
	printTrees(trees, forestWidth, forestHeight);

	for (i=0;i<forestHeight;++i) {
		if (trees[i*forestWidth] == TREE) {
			trees[i*forestWidth] = FIRE;
			queue_push(current, i*forestWidth);
		}
	}

	printTrees(trees, forestWidth, forestHeight);

	while (queue_length(current)) {
		queue_clear(next);
		while (queue_length(current)) {
			long index = queue_pop(current);
			long dx, dy, minx, miny, maxx, maxy, x, y;
			y = index / forestWidth;
			x = index % forestWidth;
			minx = x > 0 ? -1 : 0;
			miny = y > 0 ? -1 : 0;
			maxx = x < forestWidth -1 ? 1 : 0;
			maxy = y < forestHeight -1 ? 1 : 0;
			trees[index] = EMPTY;
			for (dx=minx;dx<=maxx;++dx) {
				for (dy=miny;dy<=maxy;++dy) {
					if (dx && !dy || dy && !dx) {
						long adj = index + dx + dy * forestWidth;
						if (trees[adj] == TREE) {
							trees[adj] = FIRE;
							queue_push(next, adj);
						}
					}
				}
			}
		}
		printTrees(trees, forestWidth, forestHeight);
		swap((void **)&current, (void **)&next);
	}
}


