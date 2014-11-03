#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

#define EMPTY '-'
#define TREE 'T'
#define FIRE '*'

QUEUE_MAKE(long, Queue)

void printTrees(char *trees, long width, long height) {
	printf("\x1B[1;1H");

	long total = width * height;
	long i;
	for (i=0;i<total;) {
		printf("%c", trees[i]);
		if (++i % width == 0) printf("\n");
	}
}

void generateForest(char *trees, long width, long height, float probability) {
	long i;
	long length = width*height;
	long chosenCount = (long)(probability*length);

	memset(trees, EMPTY, width * height);
	if (chosenCount == 0) {
		return;
	}

	long *chosen = malloc(sizeof(long) * chosenCount);
	for (i=0;i<chosenCount;++i) {
		chosen[i] = i;
	}

	double randPoint = 1.0 * chosenCount * RAND_MAX;
	for (i=chosenCount;i<length;++i) {
		double curRand = randPoint / (i+1);
		if (rand() < curRand) {
			chosen[rand() % chosenCount] = i;
		}
	}

	for (i=0;i<chosenCount;++i) {
		trees[chosen[i]] = TREE;
	}

	free(chosen);
}

float spanForest(Queue *current, char *trees, long width, long height) {
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
				if ((sides & 0xf) == 0xf) return 1;
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
		}
	}

	return 0;
}

float fireForest(Queue *current, char *trees, long width, long height) {
	long i;
	long count = 0;

	queue_clear(current);

	for (i=0;i<height;++i) {
		if (trees[i*width] == TREE) {
			trees[i*width] = FIRE;
			queue_push(current, i*width);
		}
	}

	while (queue_length(current)) {
		long length;
		for (i=0,length=queue_length(current);i<length;++i) {
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
		count++;
	}

	return count * 1.0 / width;
}

void parse_args(int argc, char **argv, long *forestWidth, long *forestHeight, float *start, float *end, long *levels, long *trials) {
	int argnum;
	for (argnum=2;argnum<argc;++argnum) {
		char *arg = argv[argnum];
		switch (arg[0]) {
			case 'd':
				sscanf(arg+1, "%ldx%ld", forestWidth, forestHeight);
				break;
			case 'r':
				sscanf(arg+1, "%f-%f", start, end);
				break;
			case 'l':
				sscanf(arg+1, "%ld", levels);
				break;
			case 't':
				sscanf(arg+1, "%ld", trials);
				break;
		}
	}
}

int main(int argc, char ** argv) {
	long forestWidth, forestHeight;
	long trials, trial;

	float start, end, probability;
	long levels, level;

	if (argc < 3) {
		fprintf(stderr, "forest (fire|span) [{tok}{value}]\n");
		fprintf(stderr, "    tok   value\n");
		fprintf(stderr, "    d     {width}x{height}\n");
		fprintf(stderr, "    r     {start}-{end}\n");
		fprintf(stderr, "    l     {levels}\n");
		fprintf(stderr, "    t     {trials}\n");
		return 1;
	}

	parse_args(argc, argv, &forestWidth, &forestHeight, &start, &end, &levels, &trials);

	char fname[300];

	srand(time(NULL));
	snprintf(fname, 300, "%ld.%ld.%.2f.%.2f.%ld.%ld.ffd", forestWidth, forestHeight, start, end, levels, trials);

	printf("Filename %s\n", fname);

	FILE *out = fopen(fname, "w");

	char *trees;
	trees = (char *) malloc(forestWidth * forestHeight * sizeof(char));

	Queue *q;
	q = queue_create(forestHeight);

	for (level=0;level<=levels;++level) {
		probability = start+(end-start)/(levels)*level;
		float time = 0;
		for (trial=0;trial<trials;++trial) {
			generateForest(trees, forestWidth, forestHeight, probability);
			time += fireForest(q, trees, forestWidth, forestHeight);
		}
		time /= trials;
		printf("%f %f\n", probability, time);
		fprintf(out, "%f %f\n", probability, time);
	}

	free(trees);
	return 0;
}


