#include <stdlib.h>
#include <stdio.h>

static void *memory = NULL;
static size_t length = 0;

static struct header {
	void *next;
	void *prev;
	size_t length;
	bool empty;
};

typedef struct header header;

int ajmalloc_init(void) {
	length = 2000 * sizeof(int);
	memory = malloc(length);
	printf("%d, %p", length, memory);
	if (memory == NULL) {
		return 1;
	}
	header *h = memory;
	h->next = NULL;
	h->prev = NULL;
	h->length = length - sizeof(struct header);
	h->empty = true;
	return 0;
}

void *malloc(size_t size) {
	header *h = memory;
	void *ptr = NULL;
	while(h) {
		if (h->empty && h->length < size) {
			ptr = h + sizeof(struct header);

			header *next_h = h + sizeof(struct header) + size;
			next_h->prev = h;
			next_h->next = h->next;
			next_h->length = h->length - size;
			next_h->empty = true;

			h->empty = false;
			h->length = size;
			h->next = next_h;
			break;
		} 

		h = h->next;
	}
	return ptr;
}

void free(void *ptr) {
	header *h = ptr - sizeof(struct header); // only this can throw segfault, and is fine
	h->empty = true;

	// coalescing
	header *nh = h->next;
	if (nh && nh->empty) {
		h->next = nh->next;
		h->length += nh->length + sizeof(struct header);
	}

	header *ph = h->prev;
	if (h->prev && ph->empty) {
		ph->next = h->next;
		ph->length += h->length + sizeof(struct header);
	}
	return;
}

void ajmalloc_destroy(void) {
	free(memory);
	length = 0;
}
