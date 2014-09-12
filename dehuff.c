#include <stdio.h>
#include <string.h>
#include <math.h>

const int treesize = 25000;

int main(int argc, char **argv) {
	FILE* fin;
	int n;
	char treearr[treesize];
	memset(treearr, 0, treesize);

	int counts[256];
	memset(counts, 0, sizeof(counts));
	int decbits = 0;
	int letters = 0;
	int huffbits = 0;
	double theobits = 0;

	char *tree = treearr;
	char next;
	int ptr;
	int i;
	tree--;

	
	fin = fopen(argv[1], "r");
	fscanf(fin, "%d", &n);
	fread(&next, 1, 1, fin);
	for (i=0;i<n;++i) {
		char cur;
		fread(&cur, 1, 1, fin);
		ptr = 1;
		while (fread(&next, 1, 1, fin) && next != '\n') {
			ptr <<= 1;
			ptr |= next & 1;
		}
		if (ptr > treesize) return 1;
		tree[ptr] = cur;
	}

	ptr = 1;
	while (fread(&next, 1, 1, fin)) {
		if (next != '0' && next != '1') continue;
		huffbits++;
		ptr <<= 1;
		ptr |= next & 0x1;
		if (tree[ptr]) {
			letters++;
			counts[tree[ptr]]++;
			printf("%c", tree[ptr]);
			ptr = 1;
		}
	}
	printf("\n");
	decbits = letters*8;

	for (i=0;i<256;++i) {
		if (counts[i]) {
			double rat = counts[i] * 1.0 / letters;
			double bits = -log2(rat);
			theobits += counts[i] * bits;
		}
	}

	printf("Enc %d Dec %d Comp %.2f%%\n", huffbits, decbits, (decbits - huffbits) * 100.0 / decbits);
	printf("Theoretical %.2f Extra %.2f\n", theobits, huffbits - theobits);

	return 0;
}
