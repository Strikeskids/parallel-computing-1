#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef GL_PROGRAM_H
#define GL_PROGRAM_H

#define MAX_SHADERS 100

GLuint createShader(GLenum shaderType, char *shaderText);
GLuint createProgram(char **shaderFiles);

#endif

