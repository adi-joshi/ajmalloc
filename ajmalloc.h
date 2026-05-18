#ifndef AJMALLOC_H_
#define AJMALLOC_H_

int ajmalloc_init(void);
void *malloc(size_t size);
void free(void *ptr);
void ajmalloc_destroy(void);

#endif
