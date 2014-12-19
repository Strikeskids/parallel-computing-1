#include "rendertarget.h"

int initRenderTarget(RenderTarget *tg, int width, int height) {
	glGenFramebuffers(1, &tg->framebufferId);
	glGetTextures(1, &tg->textureId);
	tg->width = width;
	tg->height = height;

	glBindTexture(GL_TEXTURE_2D, tg->textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tg->width, tg->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, tg->framebufferId);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tg->textureId, 0);

	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return 0;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void prerenderToTarget(RenderTarget *tg) {
	glBindFramebuffer(GL_FRAMEBUFFER, tg->framebufferId);
	glViewport(0, 0, tg->width, tg->height);
}

void postrenderToTarget(RenderTarget *tg) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int saveTextureToFile(FILE *file, RenderTarget *tg) {
	glBindTexture(GL_TEXTURE_2D, tg->textureId);

	int bufsize = tg->width * tg->height * 3;
	char *buffer = malloc(bufsize);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);

	fprintf(file, "P6\n%d\n%d\n255\n", tg->width, tg->height);
	fwrite(buffer, 1, bufsize, file);

	free(buffer);
	glBindTexture(GL_TEXTURE_2D, 0);
}

