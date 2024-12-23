#ifndef K_LIB_H
#define K_LIB_H

/* lib.h -- defines common kernel library functions. */

#include <kern/console/console.h>

_Noreturn void panic(const char *fmt, ...);

#define assert(condition) do { \
    if(!(condition)) { \
        panic("assertion failed: " #condition); \
    } \
} while(0)

/* Include some important libc functions. */
#include <string.h>

#include <stdarg.h>

/**
 * Kernel print function.
 */
void printk(const char *fmt, ...);

void vprintk(const char *fmt, va_list va);

void *kzalloc(size_t size);
void *kzrealloc(void *ptr, size_t new_size);
void kfree(void *mem);

void *kzalloc_or_die(size_t size, const char *fmt, ...);
void *kzrealloc_or_die(void *ptr, size_t new_size, const char *fmt, ...);

#endif