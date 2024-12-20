#ifndef K_LIB_H
#define K_LIB_H

/* lib.h -- defines common kernel library functions. */

static inline _Noreturn void panic(const char *fmt, ...) { /* TODO */ }

#define assert(condition) do { \
    if(!(condition)) { \
        panic("assertion failed: %s ", #condition); \
    } \
} while(0)

#endif