// Corwin de Boor
// Mandlebrot Fractal Generator
// (c) 2014

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glut.h>
#include <glprogram.h>

#define M 600
#define N 800

#define ZOOM 2

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

int iterations = 40;

GLuint cornersId;
GLuint mandelbrotProgramId;

void



GLuint positionBuffer;
GLuint mandlebrotProgram;
GLuint iterationsUniform;
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

	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	glutSwapBuffers();
}

void initializePrograms() {
	char *mandleShaders[] = {"mandle.vert", "mandle.frag", NULL};

	mandlebrotProgram = initializeProgram(mandleShaders);
	iterationsTextureUniform = glGetUniformLocation(mandlebrotProgram, "iterationsTexture");
	iterationsUniform = glGetUniformLocation(mandlebrotProgram, "iterations");
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

void computeMandlebrot() {
	int i, j;
	for (i=0;i<N;++i) {
		for (j=0;j<M;++j) {
			double cx = rect[0] + (i * 1.0 / N - 0.5) * rect[2];
			double cy = rect[1] + (j * 1.0 / M - 0.5) * rect[3];
			double zx = 0, zy = 0;
			int it;
			for (it=0;it<iterations&&(zx*zx+zy*zy)<4;++it) {
				double ox = zx;
				double oy = zy;
				zx = ox * ox - oy * oy + cx;
				zy = 2 * ox * oy + cy;
			}
			data[j][i] = it;
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

int main(int argc, char **argv) {
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

	return 0;
}

