#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef GL_PROGRAM_H
#define GL_PROGRAM_H

GLuint createShader(GLenum shaderType, char *shaderText);
GLuint createProgram(char **shaderFiles);

#endif

