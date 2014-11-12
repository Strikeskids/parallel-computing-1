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
#define TAG_JOB_COUNT 5

#define MIN(a,b) (a>b?b:a)
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

long computeWork(long maxWork, long trials, double start, double end, long levels, long *level, long *levelWork, long *taskCap, long **works, double **probs) {
	long task;
	long work = maxWork;
	double dprob = (end-start)/(levels-1);

	if (*levelWork <= 0) {
		*levelWork = trials;
		(*level)++;
	}

	for (task=0;work>0 && *level<levels;++task) {
		long curWork = MIN(*levelWork, work);
		*levelWork -= curWork;
		work -= curWork;
		if (task >= *taskCap) {
			*taskCap = MAX(*taskCap<<1,10);
			*works = realloc(*works, *taskCap * sizeof(long));
			*probs = realloc(*probs, *taskCap * sizeof(double));
		}
		(*works)[task] = curWork;
		(*probs)[task] = start + dprob * *level;
		if (*levelWork <= 0) {
			*levelWork = trials;
			(*level)++;
		}
	}
	return task;
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

	int size, workers, worker;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	workers = size - 1;

	long forestWidth, forestHeight;
	long trials;

	double start, end;
	long levels;

	parse_args(argc, argv, &forestWidth, &forestHeight, &start, &end, &levels, &trials);
	levels++;

	char fname[300];
	snprintf(fname, 300, "%s.%ld.%ld.%.2lf.%.2lf.%ld.%ld.ffd", argv[1], forestWidth, forestHeight, start, end, levels, trials);

	fprintf(stderr, "Filename %s\n", fname);

	long totalWork = levels * trials;
	long workPerWorker = totalWork / workers + 1;

	fprintf(stderr, "Total size (workers): %d %d\n", size, workers);
	fprintf(stderr, "Total work (per worker): %ld (%ld)\n", totalWork, workPerWorker);

	long maxWork = MIN(workPerWorker, 100000000 / forestWidth / forestHeight + 3);
	long totalJobs = totalWork / maxWork + 1, jobsReceived = 0;
	long level = -1;
	long levelWork = 0;
	long taskCap = 0;
	long *works = NULL;
	double *probs = NULL;
	double *curResults = NULL;

	long dims[2];
	dims[0] = forestWidth;
	dims[1] = forestHeight;

	fprintf(stderr, "Waiting...\n");
	long tasks, waiting = 0;
	for (worker=1;worker<=workers;++worker) {
		fprintf(stderr, "\x1b[1FInitializing worker %d\n", worker);
		MPI_Send(&command, 1, MPI_CHAR, worker, TAG_WORK_TYPE, MPI_COMM_WORLD);
		MPI_Send(&dims, 2, MPI_LONG, worker, TAG_DIMENSIONS, MPI_COMM_WORLD);
		tasks = computeWork(maxWork, trials, start, end, levels, &level, &levelWork, &taskCap, &works, &probs);
		MPI_Send(&tasks, 1, MPI_LONG, worker, TAG_JOB_COUNT, MPI_COMM_WORLD);
		MPI_Send(works, tasks, MPI_LONG, worker, TAG_WORK, MPI_COMM_WORLD);
		MPI_Send(probs, tasks, MPI_DOUBLE, worker, TAG_PROBABILITY, MPI_COMM_WORLD);
		waiting++;
	}
	
	fprintf(stderr, "\x1b[1FFinished initializing workers %ld\n", waiting);

	double *results;
	results = malloc(sizeof(double) * levels);
	
	double dprob = (end-start)/(levels-1);

	fprintf(stderr, "\x1b[1FTasks %5ld: %5ld sent %5ld received\n", totalJobs, jobsReceived+waiting, jobsReceived);

	long task, source;
	MPI_Status status;
	while (waiting) {
		MPI_Recv(&tasks, 1, MPI_LONG, MPI_ANY_SOURCE, TAG_JOB_COUNT, MPI_COMM_WORLD, &status);
		jobsReceived++;
		waiting--;
		source = status.MPI_SOURCE;
		
		curResults = realloc(curResults, sizeof(double) * tasks);
		probs = realloc(probs, sizeof(double) * tasks);
		MPI_Recv(probs, tasks, MPI_DOUBLE, source, TAG_PROBABILITY, MPI_COMM_WORLD, &status);
		MPI_Recv(curResults, tasks, MPI_DOUBLE, source, TAG_RESULT, MPI_COMM_WORLD, &status);
		
		for (task=0;task<tasks;++task) {
			double prob = probs[task];
			double result = curResults[task];
			long recvLevel = (long)((prob-start)/dprob+0.5);
			results[recvLevel] += result;
		}

		tasks = computeWork(maxWork, trials, start, end, levels, &level, &levelWork, &taskCap, &works, &probs);
		if (tasks > 0) {
			waiting++;
			MPI_Send(&tasks, 1, MPI_LONG, source, TAG_JOB_COUNT, MPI_COMM_WORLD);
			MPI_Send(works, tasks, MPI_LONG, source, TAG_WORK, MPI_COMM_WORLD);
			MPI_Send(probs, tasks, MPI_DOUBLE, source, TAG_PROBABILITY, MPI_COMM_WORLD);
		}
		fprintf(stderr, "\x1b[1FTasks %5ld: %5ld sent %5ld received\n", totalJobs, jobsReceived+waiting, jobsReceived);
	}

	tasks = 0;
	for (worker=1;worker<=workers;++worker) {
		MPI_Send(&tasks, 1, MPI_LONG, worker, TAG_JOB_COUNT, MPI_COMM_WORLD);
	}

	fprintf(stderr, "Sent terminate to all workers\n");

	FILE *out = fopen(fname, "w");

	for (level=0;level<levels;++level) {
		fprintf(out, "%g %g\n", level*dprob+start, results[level]/levels);
	}

	fclose(out);
	free(results);
	free(curResults);
	free(probs);

	MPI_Finalize();
	return 0;
}

void worker(int rank) {
	double (*command)(Queue *, char *, long, long);

	char commandNum;
	long dims[2];

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

	srand(time(NULL) * rank);

	char *trees;
	trees = (char *) malloc(dims[0] * dims[1] * sizeof(char));

	Queue *q;
	q = queue_create(dims[1]);

	long *trials = NULL;
	double *probs = NULL;
	double *results = NULL;

	while (1) {
		long tasks, task;
		MPI_Recv(&tasks, 1, MPI_LONG, 0, TAG_JOB_COUNT, MPI_COMM_WORLD, &status);
		if (!tasks) {
			break;
		}
		trials = realloc(trials, sizeof(long) * tasks);
		probs = realloc(probs, sizeof(double) * tasks);
		results = realloc(results, sizeof(double) * tasks);
		MPI_Recv(trials, tasks, MPI_LONG, 0, TAG_WORK, MPI_COMM_WORLD, &status);
		MPI_Recv(probs, tasks, MPI_DOUBLE, 0, TAG_PROBABILITY, MPI_COMM_WORLD, &status);
		for (task=0;task<tasks;++task) {
			long trial;
			results[task] = 0;
			for (trial=0;trial<trials[task];++trial) {
				generateForest(trees, dims[0], dims[1], probs[task]);
				results[task] += command(q, trees, dims[0], dims[1]);
			}
		}
		MPI_Send(&tasks, 1, MPI_LONG, 0, TAG_JOB_COUNT, MPI_COMM_WORLD);
		MPI_Send(probs, tasks, MPI_DOUBLE, 0, TAG_PROBABILITY, MPI_COMM_WORLD);
		MPI_Send(results, tasks, MPI_DOUBLE, 0, TAG_RESULT, MPI_COMM_WORLD);
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
		MPI_Finalize();
		return 0;
	}
}

