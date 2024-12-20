#ifndef LIBC_STRING_H
#define LIBC_STRING_H

#include <stddef.h>

void *memcpy(void *restrict dest, const void *restrict src, size_t nbytes);
void *memmove(void *dest, const void *src, size_t nbytes);
void *memset(void *s, int c, size_t n);

#endif