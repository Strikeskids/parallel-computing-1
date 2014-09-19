#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node_struct {
	unsigned char ch;
	int freq;
	int depth;
	struct Node_struct *left;
	struct Node_struct *right;
	struct Node_struct *parent;
} Node;

typedef struct BitStream_struct {
	FILE *file;
	int offset;
	char current;
} BitStream;

BitStream * bitStream(FILE *file) {
	BitStream * stream;

	stream = (BitStream *) malloc(sizeof(BitStream));
	stream->file = file;
	stream->offset = 0;
	stream->current = 0;
}

void bitStream_flush(BitStream * stream) {
	if (stream->offset) {
		fwrite(&stream->current, sizeof(unsigned char), 1, stream->file);
	}
	stream->current = 0;
	stream->offset = 0;
}

void bitStream_write(BitStream * stream, int bit) {
	if (stream->offset >= 8) {
		bitStream_flush(stream);
	}
	stream->current |= (bit & 0x1) << (stream->offset++);
}

unsigned int bitStream_multiWrite(BitStream *stream, char *data) {
	unsigned int i;
	for (i=0;data[i]>=0;++i) {
		bitStream_write(stream, data[i]);
	}
	return i;
}

Node *create(char ch, int f);
Node *heapPop(Node **heap, int *len);
void heapPush(Node **heap, int *len, Node *add);

int main(int argc, char** argv) {

	char* filename;
	char* output;
	if (argc > 2) {
		filename = argv[1];
		output = argv[2];
	} else {
		printf("Usage: huffman input output");
		return 1;
	}

	Node *heap[256];
	int heaplen = 0;
	Node nodes[256];

	int i;
	unsigned char cur;
	for (i=0;i<256;++i) {
		nodes[i].ch = i;
		nodes[i].freq = 0;
		nodes[i].left = NULL;
		nodes[i].right = NULL;
		nodes[i].depth = 1;
		nodes[i].parent = NULL;
	}

	FILE* input = fopen(filename, "r");
	setvbuf(input, NULL, _IOFBF, 0);
	while (fread(&cur, 1, 1, input)) {
		nodes[cur].freq++;
	}
	rewind(input);

	unsigned char validChars = 0;
	for (i=0;i<256;++i) {
		if (nodes[i].freq > 0) {
			heapPush(heap, &heaplen, &(nodes[i]));
			validChars++;
		}
	}

	while (heaplen > 1) {
		Node *j1 = heapPop(heap, &heaplen);
		Node *j2 = heapPop(heap, &heaplen);
		Node *joined = create(0, j1->freq + j2->freq);
		joined->left = j1;
		joined->right = j2;
		joined->parent = NULL;
		j1->parent = joined;
		j2->parent = joined;
		joined->depth = (j1->depth > j2->depth ? j1->depth : j2->depth) + 1;
		heapPush(heap, &heaplen, joined);
	}

	int bufsize = heap[0]->depth;
	char *buffer = (char *) malloc(bufsize + 1);
	buffer[bufsize] = 0;

	char *encoded[256];
	BitStream * outputStream;
	outputStream = bitStream(fopen(output, "w"));
	setvbuf(outputStream->file, NULL, _IOFBF, 0);
	fwrite(&validChars, sizeof(unsigned char), 1, outputStream->file);
	for (i=0;i<256;++i) {
		if (nodes[i].freq > 0) {
			Node *node = &nodes[i];
			int len = bufsize;
			unsigned short count;
			while (node->parent) {
				Node *parent = node->parent;
				buffer[--len] = (parent->left == node ? 0 : 1);
				node = parent;
			}
			count = bufsize-len;
			encoded[i] = (char *) malloc(count + 1);
			encoded[i][count] = -1;
			memcpy(encoded[i], buffer+len, count);

			bitStream_flush(outputStream);
			fwrite(&nodes[i].ch, sizeof(char), 1, outputStream->file);
			fwrite(&count, sizeof(unsigned short), 1, outputStream->file);
			bitStream_multiWrite(outputStream, encoded[i]);

		} else {
			encoded[i] = NULL;
		}
	}

	bitStream_flush(outputStream);
	long long totalBits = 0;
	long bitSizeLoc = ftell(outputStream->file);
	fwrite(&totalBits, sizeof(long long), 1, outputStream->file);

	while (fread(&cur, 1, 1, input)) {
		totalBits += bitStream_multiWrite(outputStream, encoded[cur]);
	}

	bitStream_flush(outputStream);
	fseek(outputStream->file, bitSizeLoc, SEEK_SET);
	fwrite(&totalBits, sizeof(long long), 1, outputStream->file);

	fclose(outputStream->file);
	fclose(input);

	return 0;
}

// zero indexed heap
void heapPush(Node **heap, int *len, Node *added) {
	int ind = (*len)++;
	heap[ind] = added;
	while (ind > 0) {
		int parent = (ind-1) / 2;
		if (heap[parent]->freq > heap[ind]->freq) {
			Node *tmp = heap[parent];
			heap[parent] = heap[ind];
			heap[ind] = tmp;
			ind = parent;
		} else {
			break;
		}
	}
}

// zero indexed heap
Node *heapPop(Node **heap, int *len) {
	Node *popped = heap[0];
	heap[0] = heap[--(*len)];
	int ind = 0;
	while (1) {
		int c1 = ind * 2+1, c2 = ind*2+2, next = ind;
		if (c1 < *len && heap[c1]->freq < heap[next]->freq) {
			next = c1;
		}
		if (c2 < *len && heap[c2]->freq < heap[next]->freq) {
			next = c2;
		}
		if (next == ind) break;

		Node *tmp = heap[next];
		heap[next] = heap[ind];
		heap[ind] = tmp;
		ind = next;
	}
	return popped;
}

Node *create(char ch, int f) {
	Node *node = (Node*)malloc(sizeof(Node));

	node->ch = ch;
	node->freq = f;
	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;
	node->depth = 1;

	return node;
}
