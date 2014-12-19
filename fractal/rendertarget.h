#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

typedef struct RenderTarget_struct {
	GLuint framebufferId;
	GLuint textureId;
	int width;
	int height;
} RenderTarget;

int initRenderTarget(RenderTarget *tg, int width, int height);
void prerenderToTarget(RenderTarget *tg);
void postrenderToTarget(RenderTarget *tg);
int saveTextureToFile(FILE *file, RenderTarget *tg);

#endif

