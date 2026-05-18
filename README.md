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
