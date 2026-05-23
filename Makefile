all: build

build: ajmalloc.o
	$(CC) $(CFLAGS) ajmalloc.o --shared -o libajmalloc.so

debug:
	$(CC) $(CFLAGS) -g -DDEBUG=1 ajmalloc.c --shared -o libajmalloc.so
test: build
	gcc -L. -lajmalloc tests.c -o tests

run_test: test
	LD_LIBRARY_PATH=. ./tests

debug_test: debug
	gcc -L. -lajmalloc tests.c -o tests

run_debug_test: debug_test
	LD_LIBRARY_PATH=. ./tests

.PHONY: clean

clean:
	rm -f ajmalloc.o libajmalloc.so tests.o tests
