all: build

build: ajmalloc.o
	$(CC) $(CFLAGS) ajmalloc.o --shared -o libajmalloc.so

test: build
	gcc -L. -lajmalloc tests.c -o tests

run_test: test
	LD_LIBRARY_PATH=. ./tests

.PHONY: clean

clean:
	rm -f ajmalloc.o libajmalloc.so tests.o tests
