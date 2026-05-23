#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#ifndef DEBUG
#define DEBUG 0
#endif

static void *memory = NULL;
static size_t length = 0;

static struct header {
	void *next;
	void *prev;
	size_t length;
	bool empty;
};

typedef struct header header;

static const int HSIZE_PA = sizeof(struct header) / sizeof((void *) (0x0));

int ajmalloc_init(void) {
	length = 2000 * sizeof(int);
	memory = mmap(memory, length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
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
		if (h->empty && size <= h->length) {
			ptr = (void *) ( (uintptr_t)h + sizeof(struct header));

			header *next_h = (void *) ( (uintptr_t)h + sizeof(struct header) + size);
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
#if DEBUG == 1
	printf("(malloc) malloc_ptr=%p\n", ptr);
#endif
	return ptr;
}

void free(void *ptr) {
	header *h = ptr - sizeof(struct header); // only this can throw segfault, and it shouldn't
	h->empty = true;

#if DEBUG == 1
	printf("(free) ptr=%p, header=%p\n", ptr, h);
#endif

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
	munmap(memory, length);
	length = 0;
}
