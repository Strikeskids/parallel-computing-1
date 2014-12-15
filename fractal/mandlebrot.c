// Corwin de Boor
// Mandlebrot Fractal Generator
// (c) 2014

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glut.h>

#include "mpi.h"

#define SCALE(v, smin, smax, dmin, dmax) (v-smin)*(dmax-dmin)/(smax-smin)+dmax

#define M 800
#define N 600
#define ZOOM 2

#define MANDLEMAX 1000
#define MAX_SHADERS 100

const float RECT_CORNERS[] = {
	0, 0,
	0, 1,
	1, 1,
	1, 0
};

double rect[] = {
	0, 0,
	4.0, 3.0
};

int data[M][N];

int minIterations = 1;
int iterations = 40;

GLuint positionBuffer;
GLuint mandlebrotProgram;
GLuint iterationsUniform;
GLuint minIterationsUniform;
GLuint iterationsTexture;
GLuint iterationsTextureUniform;

void displayfunc(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(mandlebrotProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, iterationsTexture);
	int textureActive = 0;
	glUniform1iv(iterationsTextureUniform, 1, &textureActive);
	glUniform1iv(iterationsUniform, 1, &iterations);
	glUniform1iv(minIterationsUniform, 1, &minIterations);

	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	glutSwapBuffers();
}

void readFile(char *fname, char **data) {
	long startLoc, bytesRead, size;

	FILE *file = fopen(fname, "r");

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);

	*data = realloc(*data, size+1);
	startLoc = 0;
	while ((bytesRead = fread(*data, 1, size-startLoc, file)) && (startLoc += bytesRead) < size) {
	}
	(*data)[size] = 0;
	fclose(file);
}

GLuint createProgramFromShaders(int num, GLuint *shaders) {
	GLuint program = glCreateProgram();

	int i;
	for (i=0;i<num;++i) {
		glAttachShader(program, shaders[i]);
	}

	glLinkProgram(program);
	GLint status;
	
	glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		char *strInfoLog = malloc(sizeof(char) * (infoLogLength+1));

		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		strInfoLog[infoLogLength] = 0;

		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		free(strInfoLog);
	}

	for (i=0;i<num;++i) {
		glDetachShader(program, shaders[i]);
	}

	return program;
}

GLuint createShader(GLenum shaderType, char *shaderData) {

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderData, NULL);

	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		char *strInfoLog = malloc(sizeof(char) * (infoLogLength+1));

		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		strInfoLog[infoLogLength] = 0;
		 
		const char *strShaderType = NULL;
		switch (shaderType) {
			case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
			case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
			case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}
		 
		fprintf(stderr, "Compile failure len %d in %s shader:\n%s\n", infoLogLength, strShaderType, strInfoLog);
		free(strInfoLog);
	} 

	return shader;
}

GLuint initializeProgram(char** shaderFiles) {
	char *shaderData = NULL;

	GLuint shaders[MAX_SHADERS];
	GLuint program;

	int snum = 0;

	for (snum=0;snum<MAX_SHADERS && shaderFiles[snum]; ++snum) {
		readFile(shaderFiles[snum], &shaderData);
		GLenum shtype;
		if (strstr(shaderFiles[snum], "vert")) {
			shtype = GL_VERTEX_SHADER;
		} else if (strstr(shaderFiles[snum], "frag")) {
			shtype = GL_FRAGMENT_SHADER;
		} else {
			return -1;
		}
		shaders[snum] = createShader(shtype, shaderData);
	}

	free(shaderData);

	program = createProgramFromShaders(snum, shaders);

	for (;(snum--)>0;) {
		glDeleteShader(shaders[snum]);
	}

	return program;
}

void initializePrograms() {
	char *mandleShaders[] = {"mandle.vert", "mandle.frag", NULL};

	mandlebrotProgram = initializeProgram(mandleShaders);
	iterationsTextureUniform = glGetUniformLocation(mandlebrotProgram, "iterationsTexture");
	iterationsUniform = glGetUniformLocation(mandlebrotProgram, "iterations");
	minIterationsUniform = glGetUniformLocation(mandlebrotProgram, "minimumIterations");
}

void reloadIterationTexture() {
	glBindTexture(GL_TEXTURE_2D, iterationsTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N, M, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void createIterationTexture() {
	glGenTextures(1, &iterationsTexture);

	glBindTexture(GL_TEXTURE_2D, iterationsTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	reloadIterationTexture();
}

void initialize(int argc, char **argv) {
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(M,N);
	glutInitWindowPosition(100,50);
	glutCreateWindow("");
	glClearColor(1.0,1.0,1.0,0.0);
	glShadeModel(GL_SMOOTH);

	glViewport(0,0,(GLsizei)M,(GLsizei)N);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}

void initializeDrawRect() {
	glGenBuffers(1, &positionBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(RECT_CORNERS), RECT_CORNERS, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#define TAG_BOUNDS 0
#define TAG_COLUMN 1
#define TAG_ITERATIONS 2
#define TAG_DATA 3

int mpisize;

void computeMandlebrot() {
	int i, r;
	MPI_Status status;
	for (i=0;i<mpisize-1&&i<M;++i) {
		MPI_Send(&i, 1, MPI_INT, i+1, TAG_COLUMN, MPI_COMM_WORLD);
		MPI_Send(&iterations, 1, MPI_INT, i+1, TAG_ITERATIONS, MPI_COMM_WORLD);
		MPI_Send(rect, 4, MPI_DOUBLE, i+1, TAG_BOUNDS, MPI_COMM_WORLD);
	}
	minIterations = iterations;
	int j;
	r = 0;
	while (r < M) {
		int c;
		MPI_Recv(&c, 1, MPI_INT, MPI_ANY_SOURCE, TAG_COLUMN, MPI_COMM_WORLD, &status);
		r += 1;
		MPI_Recv(data[c], N, MPI_INT, status.MPI_SOURCE, TAG_DATA, MPI_COMM_WORLD, &status);
		for (j=0;j<N;++j) {
			if (data[c][j] < minIterations) {
				minIterations = data[c][j];
			}
		}
		if (i < M) {
			MPI_Send(&i, 1, MPI_INT, status.MPI_SOURCE, TAG_COLUMN, MPI_COMM_WORLD);
			MPI_Send(&iterations, 1, MPI_INT, status.MPI_SOURCE, TAG_ITERATIONS, MPI_COMM_WORLD);
			MPI_Send(rect, 4, MPI_DOUBLE, status.MPI_SOURCE, TAG_BOUNDS, MPI_COMM_WORLD);
			i++;
		}
	}
}

void recalculate() {
	computeMandlebrot();
	reloadIterationTexture();
	glutPostRedisplay();
}

void keyfunc(unsigned char key, int xscr, int yscr) {
	switch (key) {
		case 'i':
			iterations *= 2;
			break;
		case 'o':
			iterations /= 2;
			if (iterations < 1)
				iterations = 1;
			break;
		default:
			return;
	}
	recalculate();
}

void zoomImage(int dir, double xp, double yp) {
	double x, y, z;
	x = (xp / M - 0.5) * rect[2] + rect[0];
	y = (0.5 - yp / N) * rect[3] + rect[1];
	z = ZOOM;
	if (dir > 0) {
		z = 1/z;	
	}
	rect[0] = (1-z)*x + z*rect[0];
	rect[1] = (1-z)*y + z*rect[1];
	rect[2] *= z;
	rect[3] *= z;
}

void mouse(int button, int state, int x, int y) {
	if (button == 0 || button == 2) {
		if (state == GLUT_DOWN) {
			zoomImage(button == 0 ? 1 : -1, x, y);
			recalculate();
		}
	}
}

void manager(int argc, char **argv) {
	initialize(argc, argv);
	initializePrograms();
	initializeDrawRect();
	computeMandlebrot();
	createIterationTexture();

	printf("%s\n", glGetString(GL_VERSION));

	glutDisplayFunc(displayfunc);
	glutKeyboardFunc(keyfunc);
	glutMouseFunc(mouse);

	glutMainLoop();
}

void worker() {
	unsigned int *coldata = data[0];
	int c, r;
	
	MPI_Status status;
	while (1) {
		MPI_Recv(&c, 1, MPI_INT, 0, TAG_COLUMN, MPI_COMM_WORLD, &status);
		if (c < 0) {
			break;
		}
		MPI_Recv(&iterations, 1, MPI_INT, 0, TAG_ITERATIONS, MPI_COMM_WORLD, &status);
		MPI_Recv(rect, 4, MPI_DOUBLE, 0, TAG_BOUNDS, MPI_COMM_WORLD, &status);
		double x;
		for (r=0;r<N;++r) {
			double cx = rect[0] + (r * 1.0 / N - 0.5) * rect[2];
			double cy = rect[1] + (c * 1.0 / M - 0.5) * rect[3];
			double zx = 0, zy = 0;
			int it;
			for (it=0;it<iterations&&(zx*zx+zy*zy)<4;++it) {
				double ox = zx;
				double oy = zy;
				zx = ox * ox - oy * oy + cx;
				zy = 2 * ox * oy + cy;
			}
			coldata[r] = it;
		}
		MPI_Send(&c, 1, MPI_INT, 0, TAG_COLUMN, MPI_COMM_WORLD);
		MPI_Send(coldata, N, MPI_INT, 0, TAG_DATA, MPI_COMM_WORLD);
	}

}


int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpisize);

	if (rank == 0) {
		manager(argc, argv);
	} else {
		worker(rank);
		MPI_Finalize();
		return 0;
	}

	return 0;
}

