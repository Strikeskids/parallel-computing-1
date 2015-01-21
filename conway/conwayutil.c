#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "conwayutil.h"
#include "tags.h"
#include "mpi.h"

char **allocConway(int width, int height) {
	char **c;
	void *mem;
	int ptrsize, arrsize;
	int r;

	ptrsize = height*sizeof(char *);
	arrsize = height*width;
	c = mem = malloc(ptrsize + arrsize);
	c[0] = (char *)(mem + ptrsize);
	for (r=1;r<height;++r) {
		c[r] = c[r-1]+width;
	}
	memset(c[0], 0, arrsize);
	return c;
}

