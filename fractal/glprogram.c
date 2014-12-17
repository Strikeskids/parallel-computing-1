#include "glprogram.h"

void readFile(char *fname, char **data);
GLuint createProgramFromShaders(int numShaders, GLuint *shaders);

GLuint createProgram(char** shaderFiles) {
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

