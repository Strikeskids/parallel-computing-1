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
	if (q->end < q->start && (q->end + 1) % q->length >= q->start) {
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

void generateForest(char *trees, long width, long height, float probability) {
	float randPoint = probability + RAND_MAX * probability;
	
	memset(trees, EMPTY, width * height);

	for (i=0,len=width*height;i<len;++i) {
		if (rand() < randPoint) {
			trees[i] = TREE;
		}
	}
}

long fireForest(char *trees, long width, long height) {

	long count = 1;

	Queue *current, *next;
	current = queue_init(height);
	next = queue_init(height);

	for (i=0;i<height;++i) {
		if (trees[i*width] == TREE) {
			trees[i*width] = FIRE;
			queue_push(current, i*width);
		}
	}

	while (queue_length(current)) {
		queue_clear(next);
		while (queue_length(current)) {
			long index = queue_pop(current);
			long dx, dy, minx, miny, maxx, maxy, x, y;
			y = index / width;
			x = index % width;
			minx = x > 0 ? -1 : 0;
			miny = y > 0 ? -1 : 0;
			maxx = x < width -1 ? 1 : 0;
			maxy = y < height -1 ? 1 : 0;
			trees[index] = EMPTY;
			for (dx=minx;dx<=maxx;++dx) {
				for (dy=miny;dy<=maxy;++dy) {
					if (dx & dy && !(dx | dy)) {
						long adj = index + dx + dy * width;
						if (trees[adj] == TREE) {
							trees[adj] = FIRE;
							queue_push(next, adj);
						}
					}
				}
			}
		}
		swap((void **)&current, (void **)&next);
		count++;
	}

	free(current);
	free(next);

	return count;
}

int main(int argc, char ** argv) {
	float probability;
	probability = 0.7;

	char *trees;

	long i, len;



	trees = (char *) malloc(forestWidth * forestHeight * sizeof(char));

	generateForest(trees, forestWidth, forestHeight, probability);
	fireForest(trees, forestWidth, forestHeight);

	free(trees);
}


