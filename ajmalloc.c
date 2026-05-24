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

static int max(int a, int b) {
	return a > b ? a : b;
}

int ajmalloc_init(void) {
	length = 2000 * sizeof(int);
	memory = mmap(memory, length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (memory == MAP_FAILED) {
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
	header *prev = NULL;
	while(h) {
		printf("(malloc) memory=%p, h=%p, empty=%d, length=%d\n", memory, h, h->empty, h->length);
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

		prev = h;
		h = h->next;
	}

	if (ptr == NULL) { // no space in current list
		int additional = max(2 * size, length) + sizeof(struct header);
		int new_length = length + additional;
		printf("(malloc) old mem start=%p\n", memory);
		void * new_memory = mmap(memory, new_length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		printf("(malloc) new mem start=%p\n", new_memory);
		if (new_memory == MAP_FAILED) return NULL;

		header *h = NULL;

		if ( (uintptr_t)new_memory < (uintptr_t)memory) { // memory extended downwards
			h = new_memory;
			header *first_h = memory;
			if (first_h->empty) {
				h->next = first_h->next;
				h->length = first_h->length + sizeof(struct header) + additional;
				h->prev = NULL;
				h->empty = true;
			} else {
				h->next = first_h;
				h->length = additional - sizeof(struct header);
				h->prev = NULL;
				h->empty = true;
			}
		} else {

			if (prev->empty) {
				prev->length += additional;
				h = prev;
			} else {
				prev->next = (void *) ( (uintptr_t) memory + length);
				header *next = prev->next;
				next->length = new_length - sizeof(struct header);
				next->empty = true;
				next->prev = prev;
				h = next;
			}
		}

		memory = new_memory;
		ptr = (void *) ( (uintptr_t)h + sizeof(struct header));

		header *next_h = (void *) ( (uintptr_t)h + sizeof(struct header) + size);
		next_h->prev = h;
		next_h->next = h->next;
		next_h->length = h->length - size - sizeof(struct header);
		next_h->empty = true;

		h->empty = false;
		h->length = size;
		h->next = next_h;
		printf("(malloc) malloc_ptr=%p, memory=%p, h=%p, empty=%d, length=%d\n", ptr, memory, h, h->empty, h->length);
		length = new_length;
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
		printf("(free) ptr=%p, header=%p, size=%d, empty=%d, nh=%p, nhsize=%d, nhempty=%d\n", ptr, h, h->length, h->empty, nh, nh->length, nh->empty);
	}

	header *ph = h->prev;
	if (h->prev && ph->empty) {
		ph->next = h->next;
		ph->length += h->length + sizeof(struct header);
		printf("(free) ptr=%p, header=%p, size=%d, empty=%d\n", ptr, ph, ph->length, ph->empty);
	}
	return;
}

void ajmalloc_destroy(void) {
	munmap(memory, length);
	length = 0;
}
