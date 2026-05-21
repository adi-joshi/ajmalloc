#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

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
	memory = mmap(memory, length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	printf("In ajmalloc: length=%d, memory=%p\n", length, memory);
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
	if (memory == NULL) return NULL;

	header *h = memory;
	void *ptr = NULL;
	while(h) {
		printf("(malloc) ptr=%p, empty=%d, length=%d\n", h + sizeof(struct header), h->empty, h->length);
		if (h->empty && size < h->length) {
			ptr = h + sizeof(struct header);

			header *next_h = h + sizeof(struct header) + size;
			next_h->prev = h;
			next_h->next = h->next;
			next_h->length = h->length - size - sizeof(struct header);
			next_h->empty = true;

			h->empty = false;
			h->length = size;
			h->next = next_h;
			break;
		} 

		h = h->next;
	}
	printf("(malloc) malloc_ptr=%p\n", ptr);
	return ptr;
}

void free(void *ptr) {
	header *h = ptr - sizeof(struct header); // only this can throw segfault, and it shouldn't
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
	printf("(free) ptr=%p, empty=%d, length=%d, prev=%p, next=%p\n", ptr, h->empty, h->length, h->prev, h->next);
	return;
}

void ajmalloc_destroy(void) {
	munmap(memory, length);
	length = 0;
}
