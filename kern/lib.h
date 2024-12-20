#ifndef K_LIB_H
#define K_LIB_H

/* lib.h -- defines common kernel library functions. */

#include <kern/console/console.h>

static inline _Noreturn void panic(const char *fmt, ...) { for(;;) { console_putstr(fmt); }}

#define assert(condition) do { \
    if(!(condition)) { \
        panic("assertion failed: " #condition); \
    } \
} while(0)

/* Include some important libc functions. */
#include <string.h>

/**
 * Kernel print function.
 */
void printk(const char *fmt, ...);

#endif