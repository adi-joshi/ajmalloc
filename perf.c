#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <dlfcn.h>

// alloc_large() measures the time required for a large amount of allocations.
uint64_t alloc_large(void) {
	struct timespec start, stop;

	size_t allocs = 100000;
	int *arr[allocs];
	// refers https://stackoverflow.com/a/10192994
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (int i = 0; i < allocs; i++) {
		arr[i] = malloc(100 * sizeof(int));
	}
	clock_gettime(CLOCK_MONOTONIC_RAW, &stop);

	for (int i = 0; i < allocs; i++) {
		free(arr[i]);
	}

	uint64_t timeus = (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_nsec - start.tv_nsec) / 1000;
	return timeus;
}


// alloc_dealloc_large measures the time required for a large amount of
// allocs and deallocs
uint64_t alloc_dealloc_large(void) {
	struct timespec start, stop;

	size_t allocs = 100000;
	int *arr[allocs];
	// refers https://stackoverflow.com/a/10192994
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (int i = 0; i < allocs; i++) {
		arr[i] = malloc(100 * sizeof(int));
	}

	for (int i = 0; i < allocs; i++) {
		free(arr[i]);
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &stop);

	uint64_t timeus = (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_nsec - start.tv_nsec) / 1000;

	return timeus;
}

// measures the amount of time for allocs and deallocs happening in a random
// order
int alloc_dealloc_random(void) {
	struct timespec start, stop;

	size_t allocs = 100000;
	int *arr[allocs];
	// refers https://stackoverflow.com/a/10192994
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (int i = 0; i < allocs; i++) {
		arr[i] = malloc(100 * sizeof(int));
	}

	for (int i = 0; i < allocs; i++) {
		free(arr[i]);
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &stop);

	uint64_t timeus = (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_nsec - start.tv_nsec) / 1000;

	return timeus;
}

int main(int argc, char **argv) {
	printf("argc = %d\n", argc);
	if (argc > 2) {
		exit(1);
	}

	int (*init_fn)(void) = NULL;
	void * (*malloc)(size_t) = malloc;
	void (*free)(void *) = free;
	void (*destroy_fn)(void) = NULL;

	if (argc == 2) {
		if (strcmp(argv[0], "ajmalloc")) {
			void *handle = dlopen("./libajmalloc.so", RTLD_LAZY);
			if (!handle) {
				printf("Could not load libajmalloc\n");
				exit(1);
			}
			
			int (*init_fn)(void) = dlsym(handle, "ajmalloc_init");
			void * (*malloc)(size_t) = dlsym(handle, "malloc");
			void (*free)(void *) = dlsym(handle, "free");
			void (*destroy_fn)(void) = dlsym(handle, "ajmalloc_destroy");
		}
	}

	int pid;
	if ((pid = fork()) == 0) {
		uint64_t val = 0;
		for (int i = 0; i < 100; i++) {
			if (init_fn) {
				init_fn();
			}
			val += alloc_large();
			if (destroy_fn) {
				destroy_fn();
			}
		}
		printf("[alloc_large] = %d.%3d\n", val / 1000, val % 1000);
		exit(0);
	}
	int status;
	waitpid(pid, &status, 0);

	if ((pid = fork()) == 0) {
		uint64_t val = 0;
		for (int i = 0; i < 100; i++) {
			if (init_fn) {
				init_fn();
			}
			val += alloc_dealloc_large();
			if (destroy_fn) {
				destroy_fn();
			}
		}
		printf("[alloc_dealloc_large] = %d.%3d\n", val / 1000, val % 1000);
		exit(0);
	}
	waitpid(pid, &status, 0);
	return 0;
}
