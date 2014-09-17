#include <stdio.h>
#include <string.h>
#include <math.h>

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

void readLetter(BitStream * stream, Node * root) {
	char ch;
	unsigned short bitSize;
	unsigned short curBit;

	fread(&ch, sizeof(char), 1, file);
	fread(&bitSize, sizeof(short), 1, file);
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
	unsigned short numChars;
	unsigned short curChar;
	Node * root, *cur, *next;
	root = (Node *) malloc(sizeof(Node));
	root->c = 0;
	root->left = NULL;
	root->right = NULL;
	fread(&numChars, sizeof(short), 1, file);
	BitStream * stream = bitStream(file);
	for (curChar=0; curChar < numChars; curChar++) {
		readLetter(stream, root);
	}
	return root;
}

BitStream *bitStream(FILE *file) {
	BitStream *stream;
	stream = (BitStream *) malloc(sizeof(BitStream));
	stream->file = file;
	bitStream_reset(stream);

	return stream;
}

void bitStream_reset(BitStream *stream) {
	stream->index = -1;
}

int bitStream_read(BitStream *stream) {
	if (0 < stream->index || stream->index >= 8) {
		stream->index = 0;
		if (!fread(&stream->value, 1, 1, stream->file)) {
			return -1;
		}
	}
	return (stream->value >> stream->index) & 1
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
	unsigned long long bits;
	unsigned long long curbit;

	int bit;

	int decbits = 0;
	int letters = 0;
	int huffbits = 0;
	double theobits = 0;

	fin = fopen(argv[1], "r");
	setvbuf(fin, NULL, _IOFBF, 0);
	root = readTree(fin);

	fout = fopen(argv[2], "w");
	setvbuf(fin, NULL, _IOFBF, 0);

	fread(&bits, sizeof(long long), 1, fin);
	BitStream * bits = bitStream(fin);

	tree = root;
	for (curbit=0;curbit<bits;++curbit) {
		huffbits++;
		
		bit = bitStream_read(bits);
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
