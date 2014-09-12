#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
	char ch;
	int freq;
	int depth;
	struct Node *left;
	struct Node *right;
	struct Node *parent;
} CharNode;

CharNode *create(char ch, int f);
CharNode *heapPop(CharNode **heap, int *len);
void heapPush(CharNode **heap, int *len, CharNode *add);

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

	CharNode *heap[256];
	int heaplen = 0;
	CharNode nodes[256];

	int i;
	char cur;
	for (i=0;i<256;++i) {
		nodes[i].ch = i;
		nodes[i].freq = 0;
		nodes[i].left = NULL;
		nodes[i].right = NULL;
		nodes[i].depth = 1;
		nodes[i].parent = NULL;
	}

	FILE* input = fopen(filename, "r");
	while (fread(&cur, 1, 1, input)) {
		nodes[cur].freq++;
	}
	fclose(input);

	int validChars = 0;
	for (i=0;i<256;++i) {
		if (nodes[i].freq > 0) {
			heapPush(heap, &heaplen, &(nodes[i]));
			validChars++;
		}
	}

	while (heaplen > 1) {
		CharNode *j1 = heapPop(heap, &heaplen);
		CharNode *j2 = heapPop(heap, &heaplen);
		CharNode *joined = create(0, j1->freq + j2->freq);
		joined->left = j1;
		joined->right = j2;
		joined->parent = NULL;
		j1->parent = joined;
		j2->parent = joined;
		joined->depth = (j1->depth > j2->depth ? j1->depth : j2->depth) + 1;
		heapPush(heap, &heaplen, joined);
	}

	input = fopen(filename, "r");

	int bufsize = heap[0]->depth;
	char *buffer = (char *) malloc(bufsize + 1);
	buffer[bufsize] = 0;

	char *encoded[256];
	FILE *outputFile;
	outputFile = fopen(output, "w");
	fprintf(outputFile, "%d\n", validChars);
	for (i=0;i<256;++i) {
		if (nodes[i].freq > 0) {
			CharNode *node = &nodes[i];
			int len = bufsize;
			while (node->parent) {
				CharNode *parent = node->parent;
				buffer[--len] = (parent->left == node ? '0' : '1');
				node = parent;
			}
			encoded[i] = (char *) malloc(bufsize - len + 1);
			sprintf(encoded[i], "%s", buffer+len);
			fprintf(outputFile, "%c%s\n", nodes[i].ch, encoded[i]);
		} else {
			encoded[i] = NULL;
		}
	}

	while (fread(&cur, 1, 1, input)) {
		fprintf(outputFile, "%s", encoded[cur]);
	}
	fprintf(outputFile, "\n");
	fclose(outputFile);

	fclose(input);

	return 0;
}

// zero indexed heap
void heapPush(CharNode **heap, int *len, CharNode *added) {
	int ind = (*len)++;
	heap[ind] = added;
	while (ind > 0) {
		int parent = (ind-1) / 2;
		if (heap[parent]->freq > heap[ind]->freq) {
			CharNode *tmp = heap[parent];
			heap[parent] = heap[ind];
			heap[ind] = tmp;
			ind = parent;
		} else {
			break;
		}
	}
}

// zero indexed heap
CharNode *heapPop(CharNode **heap, int *len) {
	CharNode *popped = heap[0];
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

		CharNode *tmp = heap[next];
		heap[next] = heap[ind];
		heap[ind] = tmp;
		ind = next;
	}
	return popped;
}

CharNode *create(char ch, int f) {
	CharNode *node = (CharNode*)malloc(sizeof(CharNode));

	node->ch = ch;
	node->freq = f;

	return node;
}
