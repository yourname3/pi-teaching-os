#ifndef K_MEM_H
#define K_MEM_H

#include <kern/types.h>

uintptr_t alloc_pages(size_t count);
void free_pages(uintptr_t addr, size_t count);

void *kzalloc(size_t size);
void kfree(void *mem);

#endif