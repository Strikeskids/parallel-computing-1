// 
// Corwin de Boor (2015)
// 
// Conway's Game of Life
// 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include <math.h>

#include "mpi.h"

#include "conwayutil.h"
#include "worker.h"
#include "tags.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define SQUARE_SIZE 3

#define MAX_WORKER_SIDE 100

int windowWidth, windowHeight, squareSize, squareWidth, squareHeight;

char **con = NULL;

int stepOnIdle=0;

typedef struct WorkerData_struct {
	int rank;
	int wr;
	int wc;
	int r;
	int c;
	int width;
	int height;
	int surrounding[4];
} WorkerData;

WorkerData *workers = NULL;
char *buffer = NULL;
int workerSide, workerCount;
int workerWidth, workerHeight;
int initialized = 0;
 
void display(void) {
	int r,c;

	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0,1.0,1.0);
	glBegin(GL_QUADS);
	for (c=0;c<squareWidth;c++) {
		for (r=0;r<squareHeight;r++) {
			if (con[r][c]) {
				glVertex2f((c+0)*squareSize, (r+0)*squareSize);
				glVertex2f((c+1)*squareSize, (r+0)*squareSize);
				glVertex2f((c+1)*squareSize, (r+1)*squareSize);
				glVertex2f((c+0)*squareSize, (r+1)*squareSize);
			}
		}
	}
	glEnd();

	glutSwapBuffers();
}

void onestep() {

	if (!initialized)
		return;
	
	int task;
	task = TASK_COMPUTE;
	MPI_Bcast(&task, 1, MPI_INT, 0, MPI_COMM_WORLD);

	task = TASK_REPORT;
	MPI_Bcast(&task, 1, MPI_INT, 0, MPI_COMM_WORLD);

	WorkerData cur;
	MPI_Status status;
	int worker, r, c;
	char *buffer = malloc((workerWidth+1)*(workerHeight+1));
	for (worker=0;worker<workerCount;++worker) {
		cur = workers[worker];
		MPI_Recv(buffer, cur.width*cur.height, MPI_CHAR, cur.rank, TAG_CONWAY_DATA, MPI_COMM_WORLD, &status);
		for (r=cur.height-1;r>=0;--r) {
			memcpy(&con[cur.r+r][cur.c], &buffer[r*cur.width], cur.width);
		}
	}
	free(buffer);
}

void idle(void) {
	if (stepOnIdle) {
		onestep();
		glutPostRedisplay();
	}
}

void mouse(int button,int state,int xscr,int yscr) {
	if (button==GLUT_LEFT_BUTTON) {
		if (state==GLUT_DOWN) {
			stepOnIdle = stepOnIdle&1 ^ 1;
		}
	}
}

void keyfunc(unsigned char key,int xscr,int yscr) {
	if (key==' ') {
		onestep();
		glutPostRedisplay();
	} else if (key=='q') {
		exit(0);
	}
}

void reshape(int wscr,int hscr) {
	windowWidth=wscr; windowHeight=hscr;
	glViewport(0,0,(GLsizei)windowWidth,(GLsizei)windowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0,windowWidth-1,0,windowHeight-1);
	glMatrixMode(GL_MODELVIEW);
}

void initializeWindow(int *argc, char* argv[]) {
	glutInit(argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(100,50);
	glutCreateWindow("Conway's Game of Life");

	glClearColor(0.0,0.0,0.0,0.0);
	glShadeModel(GL_SMOOTH);

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyfunc);
	glutReshapeFunc(reshape);
}

void initializeWorkers(int size) {
	int i, r, c, up, left;

	int worker;
	
	int heightLeft, widthLeft;
	int heights[MAX_WORKER_SIDE], widths[MAX_WORKER_SIDE];

	workerSide = (int) sqrt(size-1);
	workerCount = workerSide * workerSide;

	if (workerSide > MAX_WORKER_SIDE) {
		fprintf(stderr, "Invalid number of workers\n");
		return;
	}
	fprintf(stderr, "Valid number of workers\n");

	int workerHeightCount = workerSide, workerWidthCount = workerSide;
	workerHeight = squareHeight / workerHeightCount;
	workerWidth = squareWidth / workerWidthCount;

	fprintf(stderr, "Worker: rows %d cols %d width %d height %d\n", workerHeightCount, workerWidthCount , workerWidth, workerHeight);

	heightLeft = squareHeight;
	widthLeft = squareWidth;
	for (i=workerSide-1;i>=0;--i) {
		heights[i] = workerHeight;
		heightLeft -= heights[i];
		widths[i] = workerWidth;
		widthLeft -= widths[i];
		if (workerHeight * i < heightLeft) {
			heights[i]++;
		}
		if (workerWidth * i < widthLeft) {
			widths[i]++;
		}
	}
	fprintf(stderr, "Found worker heights\n");

	workers = malloc(workerCount * sizeof(WorkerData));

	WorkerData cur;
	cur.rank = 1;
	
	worker = 0;
	for (cur.wr=0,cur.r=0;cur.wr<workerSide;++cur.wr,cur.r+=cur.height) {
		for (cur.wc=0,cur.c=0;cur.wc<workerSide;++cur.wc,cur.c+=cur.width) {
			cur.width = widths[cur.wc];
			cur.height = heights[cur.wr];

			workers[cur.wr*workerSide+cur.wc] = cur;
			cur.rank++;
		}
	}

	fprintf(stderr, "Created workers\n");

	cur.width = 0;
	cur.height = 0;

	for (;cur.rank<size;++cur.rank) {
		MPI_Send(&cur.width, 2, MPI_INT, cur.rank, TAG_DIMENSIONS, MPI_COMM_WORLD);
	}

	fprintf(stderr, "Dismissed useless workers\n");

	for (worker=0;worker<workerCount;++worker) {
		cur = workers[worker];

		up = (cur.wr+workerSide-1)%workerSide*workerSide + cur.wc;
		left = (cur.wc+workerSide-1)%workerSide + cur.wr*workerSide;
		fprintf(stderr, "Worker %d (%d, %d) up %d left %d\n", worker, cur.wr, cur.wc, up, left);

		cur.surrounding[UP] = workers[up].rank;
		cur.surrounding[LEFT] = workers[left].rank;
		workers[up].surrounding[DOWN] = cur.rank;
		workers[left].surrounding[RIGHT] = cur.rank;

		workers[worker] = cur;
	}

	fprintf(stderr, "Found surrounding worker values\n");

	char *buffer;
	buffer = malloc((workerHeight+1)*(workerWidth+1));

	for (worker=0;worker<workerCount;++worker) {
		cur = workers[worker];

		fprintf(stderr, "Copying to worker %d: (%d, %d)\n", cur.rank, cur.width, cur.height);
		
		for (r=cur.height-1;r>=0;--r) {
			memcpy(&buffer[r*cur.width], &con[r+cur.r][cur.c], cur.width);
		}
		fprintf(stderr, "Finished copying rows\n");

		MPI_Send(&cur.width, 2, MPI_INT, cur.rank, TAG_DIMENSIONS, MPI_COMM_WORLD);
		MPI_Send(buffer, cur.width*cur.height, MPI_CHAR, cur.rank, TAG_CONWAY_DATA, MPI_COMM_WORLD);
		MPI_Send(&cur.surrounding, 4, MPI_INT, cur.rank, TAG_NEIGHBORS, MPI_COMM_WORLD);
		fprintf(stderr, "Sent data to worker\n");
	}

	fprintf(stderr, "Sent initial instructions to workers\n");
	free(buffer);
	fprintf(stderr, "Freed initial buffer\n");
}

void manager(int* argc, char* argv[], int size) {
	windowWidth = WINDOW_WIDTH;
	windowHeight = WINDOW_HEIGHT;
	squareSize = SQUARE_SIZE;
	squareWidth = windowWidth / squareSize;
	squareHeight = windowHeight / squareSize;

	FILE* fin;
	int x,y;
	char ch;

	initializeWindow(argc, argv);
	fprintf(stderr, "Initialized window\n");

	con = allocConway(squareWidth, squareHeight);

	fin=fopen("glider_gun.txt","r");
	for (y=0;y<9;y++) {
		for (x=0;x<36;x++) {
			fread(&ch,sizeof(char),1,fin);
			if (ch!='.')
				con[y][x]=1;
		}
		fread(&ch,sizeof(char),1,fin);
	}
	fclose(fin);

	fprintf(stderr, "Loaded configuration\n");
	initializeWorkers(size);
	fprintf(stderr, "Initialized workers\n");

	initialized = 1;

	glutMainLoop();

	free(workers);
}

int main(int argc, char* argv[]) {  
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (rank == 0) {
		manager(&argc, argv, size);
	} else {
		work(rank, size);
	}

	return 0;
}

