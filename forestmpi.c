#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "mpi.h"

#define EMPTY '-'
#define TREE 'T'
#define FIRE '*'

#define FIRE_ACTION 0
#define SPAN_ACTION 1

#define TAG_WORK 0
#define TAG_PROBABILITY 1
#define TAG_RESULT 2
#define TAG_WORK_TYPE 3
#define TAG_DIMENSIONS 4

#define MAX_TASKS 50

#define MAX(a,b) (a<b?b:a)

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

double spanForest(Queue *current, char *trees, long width, long height) {
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

double fireForest(Queue *current, char *trees, long width, long height) {
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

void parse_args(int argc, char **argv, long *forestWidth, long *forestHeight, double *start, double *end, long *levels, long *trials) {
	int argnum;
	for (argnum=2;argnum<argc;++argnum) {
		char *arg = argv[argnum];
		switch (arg[0]) {
			case 'd':
				sscanf(arg+1, "%ldx%ld", forestWidth, forestHeight);
				break;
			case 'r':
				sscanf(arg+1, "%lf-%lf", start, end);
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

void printHelp() {
	fprintf(stderr, "forest (fire|span) [{tok}{value}]\n");
	fprintf(stderr, "    tok   value\n");
	fprintf(stderr, "    d     {width}x{height}\n");
	fprintf(stderr, "    r     {start}-{end}\n");
	fprintf(stderr, "    l     {levels}\n");
	fprintf(stderr, "    t     {trials}\n");
}

int manager(int argc, char ** argv) {
	if (argc < 3) {
		printHelp();
		return 1;
	}

	char command;

	if (strcmp(argv[1], "fire") == 0) {
		command = FIRE_ACTION;
	} else if (strcmp(argv[1], "span") == 0) {
		command = SPAN_ACTION;
	} else {
		printHelp();
		return 1;
	}

	int size, workers;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	workers = size - 1;

	long forestWidth, forestHeight;
	long trials;

	double start, end;
	long levels;

	parse_args(argc, argv, &forestWidth, &forestHeight, &start, &end, &levels, &trials);

	char fname[300];
	snprintf(fname, 300, "%s.%ld.%ld.%.2lf.%.2lf.%ld.%ld.ffd", argv[1], forestWidth, forestHeight, start, end, levels, trials);

	fprintf(stderr, "Filename %s\n", fname);

	long totalWork = levels * trials;
	long workPerWorker = totalWork / workers + 1;

	fprintf(stderr, "Total work (per worker): %ld (%ld)\n", totalWork, workPerWorker);
	int worker;
	double dprob = (end-start)/(levels-1);
	long level = 0;
	long levelWork = trials;
	long tasks = 0;

	long dims[2];
	dims[0] = forestWidth;
	dims[1] = forestHeight;

	for (worker=1;worker<=workers;++worker) {
		long work = workPerWorker;
		long curTasks = 0;
		long curwork;
		MPI_Send(&command, 1, MPI_CHAR, worker, TAG_WORK_TYPE, MPI_COMM_WORLD);
		MPI_Send(&dims, 2, MPI_LONG, worker, TAG_DIMENSIONS, MPI_COMM_WORLD);
		while (work > 0 && level < levels && curTasks < MAX_TASKS) {
			curwork = MAX(work, levelWork);
			double prob = dprob * level + start;
			work -= curwork;
			levelWork -= curwork;
			MPI_Send(&curwork, 1, MPI_LONG, worker, TAG_WORK, MPI_COMM_WORLD);
			MPI_Send(&prob, 1, MPI_DOUBLE, worker, TAG_PROBABILITY, MPI_COMM_WORLD);
			if (levelWork <= 0) {
				levelWork = trials;
				level++;
			}
			tasks++;
			curTasks++;
		}
		curwork = 0;
		MPI_Send(&curwork, 1, MPI_LONG, worker, TAG_WORK, MPI_COMM_WORLD);
	}

	fprintf(stderr, "Total tasks sent: %ld", tasks);

	double *results;
	results = malloc(sizeof(double) * levels);
	
	long task;
	MPI_Status status;
	for (task=0;task<tasks;++task) {
		double prob;
		double result;
		MPI_Recv(&prob, 1, MPI_DOUBLE, MPI_ANY_SOURCE, TAG_PROBABILITY, MPI_COMM_WORLD, &status);
		MPI_Recv(&result, 1, MPI_DOUBLE, status.MPI_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);

		level = (long)((prob-start)/dprob+0.5);
		results[level] += result;
		fprintf(stdout, "%d %g %g\n", status.MPI_SOURCE, prob, result);
	}

	FILE *out = fopen(fname, "w");

	for (level=0;level<levels;++level) {
		fprintf(out, "%g %g", level*dprob+start, results[level]);
	}

	fclose(out);
	free(results);
}

void worker(int rank) {
	double (*command)(Queue *, char *, long, long);

	char commandNum;
	double dims[2];

	int tasks=0;
	double probs[MAX_TASKS];
	long trials[MAX_TASKS];

	MPI_Status status;
	MPI_Recv(&commandNum, 1, MPI_CHAR, 0, TAG_WORK_TYPE, MPI_COMM_WORLD, &status);
	MPI_Recv(&dims, 2, MPI_LONG, 0, TAG_DIMENSIONS, MPI_COMM_WORLD, &status);
	switch (commandNum) {
		case SPAN_ACTION:
			command = &spanForest;
			break;
		case FIRE_ACTION:
			command = &fireForest;
			break;
		default:
			return;
	}

	long work;
	double probability;
	for (tasks=0;tasks<MAX_TASKS;++tasks) {
		MPI_Recv(&work, 1, MPI_LONG, 0, TAG_WORK, MPI_COMM_WORLD, &status);
		if (work == 0) break;
		MPI_Recv(&probability, 1, MPI_DOUBLE, 0, TAG_PROBABILITY, MPI_COMM_WORLD, &status);
		probs[tasks] = probability;
		trials[tasks] = work;
	}


	srand(time(NULL) * rank);

	char *trees;
	trees = (char *) malloc(dims[0] * dims[1] * sizeof(char));

	Queue *q;
	q = queue_create(dims[1]);

	int task;
	long trial;
	for (task=0;task<tasks;++task) {
		double result;
		for (trial=0;trial<trials[task];++trial) {
			generateForest(trees, dims[0], dims[1], probs[task]);
			result += command(q, trees, dims[0], dims[1]);
		}
		MPI_Send(&probs[task], 1, MPI_DOUBLE, 0, TAG_PROBABILITY, MPI_COMM_WORLD);
		MPI_Send(&result, 1, MPI_DOUBLE, 0, TAG_RESULT, MPI_COMM_WORLD);
	}
	free(trees);
	free(q);
}

int main(int argc, char ** argv) {
	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {
		return manager(argc, argv);
	} else {
		worker(rank);
		return 0;
	}
	
}
