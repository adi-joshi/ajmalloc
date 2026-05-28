## AJMalloc

A memory allocator implementation.

### Design Decisions

### Observations

**Observation**
- For some reason, using assert calls a libc function that calls free.
As ajmalloc isn't initialized, it causes a null pointer deference in
free, causing a segfault.

**Resolution**
- Don't use asserts, just use if statements and exit to fail
loudly on errors.

### Adventure

17-05-26
- Wrote initial malloc implementation. v1 was quite easy to write.
- Took a lot of time to write the testing harness. Requirements
for testing harness were mainly to be able to run each function
in a child process (as a segfault could crash the whole testing
script, which is not desirable).

18-05-26
- Gave up on creating a generic testing framework, and just
manually wrote the code without macros / special stuff for now.
- I had overriden the default libc malloc semi-knowingly, but
all tests were segfaulting.
- Doing further digging with the coredump using gdb, the reason
was that assert was using a libc function that was calling free.
- So removed assert and just used if statements with exit, but
this was still cauing ajmalloc_init to return null pointers for
memory.
- I had used malloc in ajmalloc_init knowingly to simplify
(would convert to using mmap or sbrk later). But looking at the
code now with the malloc override + the null pointer for the
memory pointer, I realized that the malloc in ajmalloc_init was
using the overriden malloc (i.e. my malloc) to do the memory
allocation for memory, which required initialization using
ajmalloc_init!

21-05-26
- Changed malloc and free to mmap and munmap with flags that
I thought were appropriate for now.
- Realized there was one logic issue in checking if something
can be allocated to a free region (used h->length < size
instead of size < h->length)

22-05-26
- There seems to be another issue with how the header is being
calculated and recalculated, so will check that later.
- Realized that there is an issue with how I'm doing the
pointer arithmetic for header calculation. Fixed by first
casting to uintptr_t, and then doing addition subtraction
using sizeof.

23-05-26
- Also a logic error in malloc where it wouldn't
allocate if the requested size had the exact same size
as the block size, so changed that as well
- For some reason, test_init was coredumping before,
but is not now after the fixes
- Fixed a few more logic issues

24-05-26
- Realized that mmap can return an address lower than
the one specified (i.e. the heap grows downwards), so
added logic to handle that.
- The reason the size of the next or previous header
wasn't increasing during coalescing was due to not
reattaching the pointers properly during coalescing.
This is now working, and size is incrementally increasing.
- The failing test was kind of wrong as the start and
end malloced ptrs might not be the start and end of the malloc
memory segment, so changed that check to be the new malloced
pointer is the same as any pointer that was previously allocated
(this will always work due to how we are allocating memory in malloc)

27-05-26
- Tried to download and run mimalloc-bench, but wasn't able to due
to several reasons (C++ version issues, incomplete downloads,
downloading a lot of packages).
- I thought it would be a good exercise to write the performance
testing myself, so I wrote the performance testing script.
- Wrote the mass allocation and deallocation scripts. C malloc
was performing around 4x faster than ajmalloc.

28-05-26
- Wrote a random allocation and deallocation performance test in the
script. ajmalloc performs on average 1.3x better than C malloc.
