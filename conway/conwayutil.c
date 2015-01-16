#include <stdlib.h>

#include "conwayutil.h"
#include "tags.h"

char **allocConway(int width, int height) {
	char **c;
	int ptrsize, arrsize;
	int r;

	ptrsize = height*sizeof(char *);
	arrsize = height*width;
	c = malloc(ptrsize + arrsize);
	c[0] = (char *)c+ptrsize;
	for (r=1;r<height;++r) {
		c[r] = c[r-1]+width;
	}
	memset(c[0], 0, arrsize);
	return c;
}

