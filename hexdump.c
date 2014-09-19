#include <stdio.h>

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Hexdump file\n");
		return 1;
	}

	FILE *fin;
	fin = fopen(argv[1], "r");
	setvbuf(fin, NULL, _IOFBF, 0);
	unsigned char cur;
	unsigned long long current;
	current = 0;
	while (fread(&cur, 1, 1, fin)) {
		if (current%8 == 0) printf("%08llx ", current);
		printf("%02x ", cur);
		if (++current%8 == 0) printf("\n");
	}
	if (current%8 != 0) printf("\n");
	return 0;
}
