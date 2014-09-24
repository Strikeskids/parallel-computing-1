#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

typedef struct struct_Node {
	char c;
	struct struct_Node* left;
	struct struct_Node* right;
} Node;

typedef struct struct_BitStream {
	FILE *file;
	unsigned char value;
	int index;
} BitStream;

void bitStream_reset(BitStream *stream) {
	stream->index = -1;
}

BitStream * bitStream_init(FILE *file) {
	BitStream *stream;
	stream = (BitStream *) malloc(sizeof(BitStream));
	stream->file = file;
	bitStream_reset(stream);

	return stream;
}

int bitStream_read(BitStream *stream) {
	if (stream->index < 0 || stream->index >= 8) {
		stream->index = 0;
		if (!fread(&stream->value, 1, 1, stream->file)) {
			return -1;
		}
	}
	stream->index++;
	return (stream->value >> (stream->index-1)) & 1;
}


void readLetter(BitStream * stream, Node * root) {
	Node *cur;
	Node *next;
	char ch;
	unsigned char bitSize;
	unsigned char curBit;

	fread(&ch, sizeof(char), 1, stream->file);
	fread(&bitSize, sizeof(unsigned char), 1, stream->file);
	bitStream_reset(stream);
	cur = root;
	for (curBit=0;curBit<bitSize;++curBit) {
		int bit = bitStream_read(stream);
		next = bit ? cur->left : cur->right;
		if (!next) {
			next = (Node *) malloc(sizeof(Node));
			next->c = 0;
			next->left = NULL;
			next->right = NULL;
		}
		if (bit) {
			cur->left = next;
		} else {
			cur->right = next;
		}
		cur = next;
	}
	cur->c = ch;
}

Node * readTree(FILE* file) {
	unsigned char numChars;
	unsigned short curChar;
	Node * root, *cur, *next;
	root = (Node *) malloc(sizeof(Node));
	root->c = 0;
	root->left = NULL;
	root->right = NULL;
	fread(&numChars, sizeof(unsigned char), 1, file);
	BitStream * stream = bitStream_init(file);
	for (curChar=0; curChar < numChars; curChar++) {
		readLetter(stream, root);
	}
	return root;
}

int main(int argc, char **argv) {

	char *input, *output;
	if (argc < 2) {
		printf("Usage dehuff input output\n");
		return 1;
	} else {
		input = argv[1];
		output = argv[2];
	}

	FILE *fin;
	FILE *fout;
	Node *tree;
	Node *root;
	long long bits;
	unsigned long long curbit;

	int bit;

	long counts[256];
	memset(counts, 0, sizeof(counts));
	int decbits = 0;
	int letters = 0;
	int huffbits = 0;
	double theobits = 0;

	int i;

	fin = fopen(argv[1], "r");
	setvbuf(fin, NULL, _IOFBF, 0);
	root = readTree(fin);

	fout = fopen(argv[2], "w");
	setvbuf(fin, NULL, _IOFBF, 0);

	fread(&bits, sizeof(long long), 1, fin);
	BitStream * stream;
	stream = bitStream_init(fin);

	tree = root;
	for (curbit=0;curbit<bits;++curbit) {
		huffbits++;
		
		bit = bitStream_read(stream);
		if (bit < 0) return 1;
		tree = bit ? tree->left : tree->right;
		if (tree->c) {
			letters++;
			counts[tree->c]++;
			fwrite(&tree->c, sizeof(char), 1, fout);
			tree = root;
		}
	}
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
