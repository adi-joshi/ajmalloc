#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "ajmalloc.h"
#include <assert.h>


//  this implementation doesn't work as expected, but we fix it later :)
static int total_tests = 0;

// this is quite scuffed, but we fix it later :)
#define ADD_TEST(fn) total_tests++; \
	if (!run_testcase(fn, total_tests, #fn)) { return 0; }

int run_testcase(void (*fn)(void), int test_num, char *fn_name) {
	int pid = 1;
	if ((pid = fork()) == 0) {
		printf("\t\t[%d / %d] Running test %s...\n", test_num, total_tests, fn_name);
		fn();
		printf("\t\t[%d / %d] Test %s succeeded!\n", test_num, total_tests, fn_name);
		return 0;
	}
	int status;
	waitpid(pid, &status, WEXITED);
	return 1;
}

void test_init(void) {
	int ret = ajmalloc_init();
	if (ret != 0) exit(2);
	ajmalloc_destroy();
	printf("hello\n");
}

void test_alloc(void) {
	int ret = ajmalloc_init();
	if (ret != 0) exit(1);

	int *ptr = malloc(10 * sizeof(int));
	if (ptr == NULL) exit(1);
	ajmalloc_destroy();
	printf("hello\n");
}

void test_dealloc(void) {
	int ret = ajmalloc_init();
	if (ret != 0) exit(1);

	int *ptr1 = malloc(10 * sizeof(int));
	free(ptr1);
	ajmalloc_destroy();
	printf("hello\n");
}

void test_space_reuse(void) {
	int ret = ajmalloc_init();
	if (ret != 0) exit(1);

	int *ptr1 = malloc(10 * sizeof(int));
	int *ptr2 = malloc(10 * sizeof(int));
	if (!(ptr1 != ptr2)) exit(1);
	int *ref = ptr1;
	free(ptr1);
	int *ptr3 = malloc(10 * sizeof(int));
	if (ptr3 != ref) exit(1);
	ajmalloc_destroy();
	printf("hello\n");
}

int main(void) {
	int pid;
	if ((pid = fork()) == 0) {
		test_init();
		printf("Completed test_init");
		return 0;
	}
	int status;
	int ret;
	ret = waitpid(pid, &status, 0);
	printf("%d = %d\n", pid, ret);
	if (WIFEXITED(status)) printf("Exit signal: %d\n", WEXITSTATUS(status));
	if (WIFSIGNALED(status)) printf("Term signal: %d\n", WTERMSIG(status));
	if (WCOREDUMP(status)) printf("core dumped\n");
	if (WIFSTOPPED(status)) printf("Stop signal: %d\n", WSTOPSIG(status));

	/*

	if ((pid = fork()) == 0) {
		test_alloc();
		printf("Completed test_alloc");
		return 0;
	}
	ret = waitpid(pid, &status, 0);
	printf("%d = %d\n", pid, ret);
	if ((pid = fork()) == 0) {
		test_dealloc();
		printf("Completed test_dealloc");
		return 0;
	}
	ret = waitpid(pid, &status, 0);
	printf("%d = %d\n", pid, ret);
	if ((pid = fork()) == 0) {
		test_space_reuse();
		printf("Completed test_space_reuse");
		return 0;
	}
	ret = waitpid(pid, &status, 0);
	printf("%d = %d\n", pid, ret);
	*/
	return 0;
}
