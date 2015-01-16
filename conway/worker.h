#ifndef WORKER_H
#define WORKER_H

typedef struct Worker_struct {
	int rank;
	int size;
	int width;
	int height;
	int surrounding[4];
	int order[4];
} Worker;

void work(int rank, int size);

#endif

