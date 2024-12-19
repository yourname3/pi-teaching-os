#ifndef K_MEM_H
#define K_MEM_H

#include <kern/types.h>

void *kzalloc(size_t size);
void kfree(void *mem);

#endif