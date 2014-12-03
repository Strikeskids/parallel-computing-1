// Corwin de Boor
// Mandlebrot Fractal Generator
// (c) 2014

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glut.h>

#define SCALE(v, smin, smax, dmin, dmax) (v-smin)*(dmax-dmin)/(smax-smin)+dmax

#define M 800
#define N 600

#define MANDLEMAX 1000
#define MAX_SHADERS 100

const float RECT_CORNERS[] = {
	0, 0,
	0, 1,
	1, 1,
	1, 0
};

float rect[] = {
	0, 0,
	4.0, 3.0
};

int iterations = 40;

GLuint positionBuffer;
GLuint rectUniform;
GLuint iterationsUniform;
GLuint mandlebrotProgram;

void displayfunc(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(mandlebrotProgram);

	glUniform4fv(rectUniform, 1, rect);
	glUniform1iv(iterationsUniform, 1, &iterations);

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
	rectUniform = glGetUniformLocation(mandlebrotProgram, "zoomRect");
	iterationsUniform = glGetUniformLocation(mandlebrotProgram, "iterations");
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

void keyfunc(unsigned char key, int xscr, int yscr) {
	if (key == 'w') {
		int i;
		for (i=0;i<4;++i) {
			rect[i] /= 1.10;
		}
		glutPostRedisplay();
	}
}


int main(int argc, char **argv) {
	initialize(argc, argv);
	initializePrograms();
	initializeDrawRect();

	glutDisplayFunc(displayfunc);
	glutKeyboardFunc(keyfunc);

	glutMainLoop();

	return 0;
}

