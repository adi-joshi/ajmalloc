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

	srand(0);
	// 1, 2, ..., 100000
	// at each alloc, random chance to free an allocated ptr.
	// so at 1, 2, ..., n, free a random ptr that is allocated (and not freed)
	// maybe best is to create a int *free_ptrs[2 * allocs], int *shuffled[allocs]
	// then put a shuffled version of [1, 2, ..., 1000] into shuffled, then
	// roll the dice. If at position i, the dice is 0, then free_ptrs[i] = -1.
	// Otherwise free_ptrs[i] = shuffled[next_pos].
	// Then, rearrange free_ptrs, so that at position i, if free_ptrs[i] > i,
	// then replace free_ptrs[i] with the next non -1 occurrence in free_ptrs
	
	int free_ptrs[2 * allocs];

	for (int i = 0; i < 2 * allocs; i++) {
		free_ptrs[i] = -1;
	}

	for (int i = 0; i < allocs; i++) {
		// clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		int x = i + (rand() % (2 * allocs - i));
		for (int j = x; j < 2 * allocs; j++) {
			if (free_ptrs[j] == -1) {
				free_ptrs[j] = i;
				break;
			}
		}
		/*
		clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
		uint64_t timeus_setup = (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_nsec - start.tv_nsec) / 1000;
		printf("[alloc_dealloc_random setup %d -> %d] = %d.%3d\n", i, x, timeus_setup / 1000, timeus_setup % 1000);
		*/
	}
	
	/*
	int shuffled[allocs];
	for (int i = 0; i < allocs; i++) {
		shuffled[i] = i;
	}

	// now shuffle
	
	for (int i = 0; i < allocs; i++) {
		int x = rand() % allocs; // biased, but should be ok
		int temp = shuffled[i];
		shuffled[i] = shuffled[x];
		shuffled[x] = temp;
	}

	// now add to free_ptrs (if rand), else -1

	int shuffled_idx = 0;
	for (int i = 0; i < allocs; i++) {
		if (rand() >= RAND_MAX / 2) {
			free_ptrs[i] = shuffled[shuffled_idx];
			shuffled_idx++;
		} else {
			free_ptrs[i] = -1;
		}
	}

	// add remaining not added previously
	
	for (int i = 0; i < allocs - shuffled_idx; i++) {
		free_ptrs[allocs + i] = shuffled[shuffled_idx + i];
	}

	// -1 for the end
	for (int i = 2 * allocs - shuffled_idx; i < 2 * allocs; i++) {
		free_ptrs[i] = -1;
	}

	for (int i = 0; i < 2 * allocs; i++) {
		if (free_ptrs[i] == -1) continue;

		if (free_ptrs[i] > i) {
			for (int j = i; j < 2 * allocs; j++) {
				if (free_ptrs[j] == -1) continue;

				if (free_ptrs[j] <= i) {
					int temp = free_ptrs[i];
					free_ptrs[i] = free_ptrs[j];
					free_ptrs[j] = temp;
				}
			}
		}
	}
	*/

	// now free_ptrs should contain the sequence of frees
	/*
	printf("[");
	for (int i = 0; i < 2 * allocs; i++) {
		printf("%d, ", free_ptrs[i]);
	}
	printf("]\n");
	*/

	// check for duplicates or inconsistencies
	int check[allocs];
	for (int i = 0; i < allocs; i++) {
		check[i] = -1;
	}

	for (int i = 0; i < 2 * allocs; i++) {
		if (free_ptrs[i] == -1) {
			continue;
		}

		if (check[free_ptrs[i]] != -1) {
			printf("Not consistent at index %d\n", i);
			exit(1);
		}
		check[free_ptrs[i]] = free_ptrs[i];
	}

	// refers https://stackoverflow.com/a/10192994
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (int i = 0; i < allocs; i++) {
		arr[i] = malloc(100 * sizeof(int));
		if (free_ptrs[i] != -1) {
			free(arr[free_ptrs[i]]);
		}
	}

	for (int i = allocs; i < 2 * allocs; i++) {
		if (free_ptrs[i] != -1) {
			free(arr[free_ptrs[i]]);
		}
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &stop);

	uint64_t timeus = (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_nsec - start.tv_nsec) / 1000;

	return timeus;
}

int main(int argc, char **argv) {
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
		printf("Malloc impl = ajmalloc\n");
	} else {
		printf("Malloc impl = C malloc\n");
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
		printf("[alloc_large] = %d.%03d\n", val / 1000, val % 1000);
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
		printf("[alloc_dealloc_large] = %d.%03d\n", val / 1000, val % 1000);
		exit(0);
	}
	waitpid(pid, &status, 0);

	if ((pid = fork()) == 0) {
		uint64_t val = 0;
		for (int i = 0; i < 10; i++) {
			if (init_fn) {
				init_fn();
			}
			val += alloc_dealloc_random();
			if (destroy_fn) {
				destroy_fn();
			}
		}
		printf("[alloc_dealloc_random] = %d.%03d\n", val / 1000, val % 1000);
		exit(0);
	}
	waitpid(pid, &status, 0);
	return 0;
}
