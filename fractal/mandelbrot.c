// Corwin de Boor
// Mandlebrot Fractal Generator
// (c) 2014

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glut.h>

#include "glprogram.h"
#include "rendertarget.h"

#define windowHeight 600
#define windowWidth 800

#define ZOOM 2

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

int iterations = 40;

GLuint cornersId;

GLuint mandelbrotProgramId;
GLuint iterationsUniformId;
GLuint boundsUniformId;

void drawMandlebrot() {
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(mandelbrotProgramId);

	glUniform1iv(iterationsUniformId, 1, &iterations);
	glUniform4dv(boundsUniformId, 1, rect);

	glBindBuffer(GL_ARRAY_BUFFER, cornersId);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void displayfunc(void) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	drawMandlebrot();
	glutSwapBuffers();
}

void initializePrograms() {
	char *mandelShaders[] = {"mandel.vert", "mandel.frag", NULL};

	mandelbrotProgramId = createProgram(mandelShaders);
	
	iterationsUniformId = glGetUniformLocation(mandelbrotProgramId, "iterations");
	boundsUniformId = glGetUniformLocation(mandelbrotProgramId, "bounds");
}

void initialize(int argc, char **argv) {
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(100, 50);
	glutCreateWindow("");
	glClearColor(1.0,1.0,1.0,0.0);
	glShadeModel(GL_SMOOTH);

	glViewport(0, 0, windowWidth, windowHeight);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}

void initializeCorners() {
	glGenBuffers(1, &cornersId);

	glBindBuffer(GL_ARRAY_BUFFER, cornersId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(RECT_CORNERS), RECT_CORNERS, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void recalculate() {
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
	x = (xp / windowHeight - 0.5) * rect[2] + rect[0];
	y = (0.5 - yp / windowWidth) * rect[3] + rect[1];
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

void render() {
	RenderTarget rt;
	initRenderTarget(&rt, windowWidth, windowHeight);
	prerenderToTarget(&rt);
	drawMandlebrot();
	postrenderToTarget(&rt);
	FILE *output = fopen("frame.ppm", "w");
	saveTextureToFile(output, &rt);
	fclose(output);
}

int main(int argc, char **argv) {
	initialize(argc, argv);
	initializePrograms();
	initializeCorners();

	printf("%s\n", glGetString(GL_VERSION));

	glutDisplayFunc(displayfunc);
	glutKeyboardFunc(keyfunc);
	glutMouseFunc(mouse);

	render();

	glutMainLoop();

	return 0;
}

