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
	q->length = length+1;
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
	if ((q->end + 1) % q->length == q->start) {
		queue_extend(q, q->length * 2 - 1);
	}
	q->items[q->end++] = item;
	q->end %= q->length;
}

long queue_pop(Queue *q) {
	long ans = q->items[q->start++];
	q->start %= q->length;
	return ans;
}

void queue_clear(Queue *q) {
	q->start = 0;
	q->end = 0;
}

long queue_get(Queue *q, long n) {
	return q->items[(q->start + n) % q->length];
}

void generateForest(char *trees, long width, long height, float probability) {
	long i;
	long length = width*height;
	long chosenCount = (long)(probability*length);
	memset(trees, EMPTY, width * height);

	if (chosenCount == 0) {
		return;
	}

	float randPoint;

	long *chosen = malloc(sizeof(long) * chosenCount);
	for (i=0;i<chosenCount;++i) {
		chosen[i] = i;
	}
	
	double cbase = 1.0 * chosenCount * RAND_MAX;

	for (i=chosenCount;i<length;++i) {
		if (rand() < cbase / (i+1)) {
			chosen[rand() % chosenCount] = i;
		}
	}

	for (i=0;i<chosenCount;++i) {
		trees[chosen[i]] = TREE;
	}
	free(chosen);
}

long spanForest(Queue *current, char *trees, long width, long height) {
	long i;

	long count = 0;

	queue_clear(current);

	for (i=0;i<height;++i) {
		if (trees[i*width] == TREE) {
			trees[i*width] = FIRE;
			queue_push(current, i*width);
			long sides = 0;
			while (queue_length(current)) {
				long index = queue_pop(current);
				long dx, dy, minx, miny, maxx, maxy, x, y;
				y = index / width;
				x = index % width;
				if (y == 0) sides |= 0x1;
				if (x == 0) sides |= 0x2;
				if (y == height-1) sides |= 0x4;
				if (x == width -1) sides |= 0x8;
				minx = x > 0 ? -1 : 0;
				miny = y > 0 ? -1 : 0;
				maxx = x < width -1 ? 1 : 0;
				maxy = y < height -1 ? 1 : 0;
				for (dx=minx;dx<=maxx;++dx) {
					for (dy=miny;dy<=maxy;++dy) {
						if (dx && !dy || !dx && dy) {
							long adj = index + dx + dy * width;
							if (trees[adj] == TREE) {
								trees[adj] = FIRE;
								queue_push(current, adj);
							}
						}
					}
				}
			}
			if ((sides & 0xf) == 0xf) {
				return 1;
			}
		}
	}

	return 0;
}

long spanForest_noQueue(char *trees, long width, long height) {
	Queue *current;
	current = queue_init(height);
	long count = spanForest(current, trees, width, height);
	free(current);
	return count;
}


int main(int argc, char ** argv) {
	long forestWidth, forestHeight;
	long trials, trial;

	float start, end, probability;
	long levels, level;

	if (argc < 7) {
		fprintf(stderr, "forestspan width height start end levels trials\n");
		return 1;
	}

	sscanf(argv[1], "%ld", &forestWidth);
	sscanf(argv[2], "%ld", &forestHeight);
	sscanf(argv[3], "%f", &start);
	sscanf(argv[4], "%f", &end);
	sscanf(argv[5], "%ld", &levels);
	sscanf(argv[6], "%ld", &trials);

	char fname[300];

	srand(time(NULL));
	snprintf(fname, 300, "span.%ld.%ld.%.2f.%.2f.%ld.%ld.dat", forestWidth, forestHeight, start, end, levels, trials);

	printf("Filename %s\n", fname);

	FILE *out = fopen(fname, "w");

	char *trees;
	trees = (char *) malloc(forestWidth * forestHeight * sizeof(char));

	Queue *a;
	a = queue_init(forestHeight);

	for (level=0;level<=levels;++level) {
		probability = start+(end-start)/(levels)*level;
		long count = 0;
		for (trial=0;trial<trials;++trial) {
			generateForest(trees, forestWidth, forestHeight, probability);
			count += spanForest(a, trees, forestWidth, forestHeight);
		}
		float spanning = count * 1.0 / trials;
		printf("%f %f\n", probability, spanning);
		fprintf(out, "%f %f\n", probability, spanning);
	}

	free(trees);
	return 0;
}


