#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#include "tags.h"
#include "conwayutil.h"
#include "worker.h"

void findOrder(Worker *w, int i1, int i2);
void exchangeData(Worker w, char **data, int dir, int send, int recv, int len);
void exchangeSides(Worker w, char **data, int lower, int upper, int width, int height);
void accumulate(char **src, char **dest, int width, int height);

void updateOrder(Worker *w, MPI_Status status, int order, int lower, int upper) {
	if (w->surrounding[lower] == status.MPI_SOURCE) {
		w->order[lower] = order;
	} else if (w->surrounding[upper] == status.MPI_SOURCE) {
		w->order[upper] = order;
	}
}

void findOrder(Worker *w, int lower, int upper) {
	MPI_Status status;

	fprintf(stderr, "Worker %d finding order\n", w->rank);

	if (w->surrounding[lower] == w->rank || w->surrounding[upper] == w->rank) {
		fprintf(stderr, "Worker %d surrounds itself\n", w->rank);
		w->order[lower] = -1;
		w->order[upper] = -1;
		return;
	}

	int tmp, order;
	if (w->surrounding[lower] > w->surrounding[upper]) {
		tmp = lower;
		lower = upper;
		upper = tmp;
	}

	fprintf(stderr, "Worker %d comparing %d %d\n", w->rank, w->surrounding[lower], w->surrounding[upper]);
	
	if (w->rank < w->surrounding[lower]) {
		fprintf(stderr, "%d local min\n", w->rank);

		MPI_Recv(&order, 1, MPI_INT, MPI_ANY_SOURCE, TAG_ORDER_SET, MPI_COMM_WORLD, &status);
		fprintf(stderr, "%d -> %d : %d\n", status.MPI_SOURCE, w->rank, order);
		updateOrder(w, status, order, lower, upper);

		MPI_Recv(&order, 1, MPI_INT, MPI_ANY_SOURCE, TAG_ORDER_SET, MPI_COMM_WORLD, &status);
		fprintf(stderr, "%d -> %d : %d\n", status.MPI_SOURCE, w->rank, order);
		updateOrder(w, status, order, lower, upper);

		if (w->order[lower] == w->order[upper]) {
			w->order[upper] = 2;
		}
		fprintf(stderr, "%d -> %d : %d\n", w->rank, w->surrounding[lower], w->order[lower]);
		MPI_Send(&w->order[lower], 1, MPI_INT, w->surrounding[lower], TAG_ORDER_RES, MPI_COMM_WORLD);

		fprintf(stderr, "%d -> %d : %d\n", w->rank, w->surrounding[upper], w->order[upper]);
		MPI_Send(&w->order[upper], 1, MPI_INT, w->surrounding[upper], TAG_ORDER_RES, MPI_COMM_WORLD);
	} else if (w->rank < w->surrounding[upper]) {
		fprintf(stderr, "%d on side\n", w->rank);

		MPI_Recv(&order, 1, MPI_INT, w->surrounding[upper], TAG_ORDER_SET, MPI_COMM_WORLD, &status);
		fprintf(stderr, "%d -> %d : %d\n", status.MPI_SOURCE, w->rank, order);
		w->order[upper] = order;
		order = order&1 ^ 1;

		fprintf(stderr, "%d -> %d : %d\n", w->rank, w->surrounding[lower], w->order[lower]);
		MPI_Send(&order, 1, MPI_INT, w->surrounding[lower], TAG_ORDER_SET, MPI_COMM_WORLD);

		MPI_Recv(&w->order[lower], 1, MPI_INT, w->surrounding[lower], TAG_ORDER_RES, MPI_COMM_WORLD, &status);
		fprintf(stderr, "%d -> %d : %d\n", status.MPI_SOURCE, w->rank, order);

		fprintf(stderr, "%d -> %d : %d\n", w->rank, w->surrounding[upper], w->order[upper]);
		MPI_Send(&w->order[upper], 1, MPI_INT, w->surrounding[upper], TAG_ORDER_RES, MPI_COMM_WORLD);
	} else {
		fprintf(stderr, "%d local max\n", w->rank);

		w->order[lower] = 1;
		w->order[upper] = 0;

		fprintf(stderr, "%d -> %d : %d\n", w->rank, w->surrounding[lower], w->order[lower]);
		MPI_Send(&w->order[lower], 1, MPI_INT, w->surrounding[lower], TAG_ORDER_SET, MPI_COMM_WORLD);

		fprintf(stderr, "%d -> %d : %d\n", w->rank, w->surrounding[upper], w->order[upper]);
		MPI_Send(&w->order[upper], 1, MPI_INT, w->surrounding[upper], TAG_ORDER_SET, MPI_COMM_WORLD);

		MPI_Recv(&order, 1, MPI_INT, MPI_ANY_SOURCE, TAG_ORDER_RES, MPI_COMM_WORLD, &status);
		fprintf(stderr, "%d -> %d : %d\n", status.MPI_SOURCE, w->rank, order);
		updateOrder(w, status, order, lower, upper);

		MPI_Recv(&order, 1, MPI_INT, MPI_ANY_SOURCE, TAG_ORDER_RES, MPI_COMM_WORLD, &status);
		fprintf(stderr, "%d -> %d : %d\n", status.MPI_SOURCE, w->rank, order);
		updateOrder(w, status, order, lower, upper);
	}
	fprintf(stderr, "Computed order\n");
}

void exchangeData(Worker w, char **data, int dir, int send, int recv, int len) {
	MPI_Status status;
	if (w.surrounding[dir] < w.rank) {
		MPI_Recv(data[recv], len, MPI_CHAR, w.surrounding[dir], TAG_CONWAY_DATA, MPI_COMM_WORLD, &status);
		MPI_Send(data[send], len, MPI_CHAR, w.surrounding[dir], TAG_CONWAY_DATA, MPI_COMM_WORLD);
	} else {
		MPI_Send(data[send], len, MPI_CHAR, w.surrounding[dir], TAG_CONWAY_DATA, MPI_COMM_WORLD);
		MPI_Recv(data[recv], len, MPI_CHAR, w.surrounding[dir], TAG_CONWAY_DATA, MPI_COMM_WORLD, &status);
	}
}

void exchangeSides(Worker w, char **data, int lower, int upper, int width, int height) {
	int i;
	if (w.order[lower] < 0 || w.order[upper] < 0) {
		// Self referential
		
		memcpy(data[height+1], data[1], width);
		memcpy(data[0], data[height], width);
	} else {
		for (i=0;i<3;++i) {
			if (w.order[lower] == i) {
				exchangeData(w, data, lower, 1, 0, width);
			} else if (w.order[upper] == i) {
				exchangeData(w, data, upper, height, height+1, width);
			}
		}
	}
}

void accumulate(char **src, char **dest, int width, int height) {
	int r, c;
	for (c=width-1;c>=0;--c) {
		dest[c][0] = src[0][c] + src[1][c] + src[2][c];
		for (r=1;r<height;++r) {
			dest[c][r] = dest[c][r-1] - src[r-1][c] + src[r+2][c]; 
		}
	}
}

void work(int rank, int size) {
	MPI_Status status;
	Worker w;
	int i, r, c, task;
	char **con, **coltally, **alltally;
	char *upper, *lower, *left, *right;

	w.rank = rank;
	w.size = size;

	fprintf(stderr, "Worker starting\n");

	MPI_Recv(&w.width, 2, MPI_INT, 0, TAG_DIMENSIONS, MPI_COMM_WORLD, &status);

	if (w.width <= 0 || w.height <= 0) {
		fprintf(stderr, "Bad worker sizes %d %d\n", w.width, w.height);
		MPI_Finalize();
		return;
	}
	fprintf(stderr, "Valid worker sizes %d %d\n", w.width, w.height);

	con = allocConway(w.width, w.height+2);
	coltally = allocConway(w.height, w.width+2);
	alltally = allocConway(w.width, w.height);

	MPI_Recv(con[1], w.width*w.height, MPI_CHAR, 0, TAG_CONWAY_DATA, MPI_COMM_WORLD, &status);
	MPI_Recv(&w.surrounding, 4, MPI_INT, 0, TAG_NEIGHBORS, MPI_COMM_WORLD, &status);

	fprintf(stderr, "Worker received data\n");

	findOrder(&w, UP, DOWN);
	findOrder(&w, RIGHT, LEFT);

	fprintf(stderr, "Worker found orders\n");

	while (1) {
		MPI_Bcast(&task, 1, MPI_INT, 0, MPI_COMM_WORLD);

		switch (task) {
		case TASK_COMPUTE:
			exchangeSides(w, con, UP, DOWN, w.width, w.height);
			accumulate(con, coltally+1, w.width, w.height);
			
			exchangeSides(w, coltally, LEFT, RIGHT, w.height, w.width);
			accumulate(coltally, alltally, w.height, w.width);
		
			for (r=w.height-1;r>=0;--r) {
				for (c=w.width-1;c>=0;--c) {
					alltally[r][c] -= con[r+1][c];
					con[r+1][c] = !!(alltally[r][c] == 3 || alltally[r][c] == 2 && con[r+1][c]);
				}
			}
			break;
		case TASK_REPORT:
			MPI_Send(con[1], w.width*w.height, MPI_CHAR, 0, TAG_CONWAY_DATA, MPI_COMM_WORLD);
			break;
		}
	}

	free(con);
	free(coltally);
	free(alltally);
}

