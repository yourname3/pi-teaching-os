#ifndef LIBC_STRING_H
#define LIBC_STRING_H

#include <stddef.h>

void *memcpy(void *restrict dest, const void *restrict src, size_t nbytes);
void *memmove(void *dest, const void *src, size_t nbytes);
void *memset(void *s, int c, size_t n);

#ifdef __GNUC__
/**
 * On GCC, allow it to optimize calls to the string functions by using the
 * __builtin versions. This is because functions like memcpy are pretty fundamental
 * operations (e.g. copying opaque memory) that can only be done safely through
 * this specific API.
 * 
 * Allowing the compiler to easily optimize them to register movs and other
 * simple operations where possible is what we want.
 */
#define memcpy  __builtin_memcpy
#define memmove __builtin_memmove
#define memset  __builtin_memset
#endif

#endif