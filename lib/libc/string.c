#include "string.h"

#include <stdint.h>

void*
memcpy(void *restrict dest, const void *restrict src, size_t nbytes) {
    char *dest_data = dest;
    const char *src_data = src;
    for(size_t i = 0; i < nbytes; ++i) {
        dest_data[i] = src_data[i];
    }
    return dest;
}

void*
memmove(void *dest, const void *src, size_t nbytes) {
    /* If the source pointer lies before the dest pointer, we need to copy
     * backwards if they're overlapping, so just always copy backwards
     * in that case. */

    char *dest_data = dest;
    const char *src_data = src;

    if((uintptr_t)src < (uintptr_t)dest) {
        for(size_t i = nbytes; i --> 0;) {
            dest_data[i] = src_data[i];
        }
    }
    else {
        /* Otherwise, copy forward. */
        for(size_t i = 0; i < nbytes; ++i) {
            dest_data[i] = src_data[i];
        }
    }

    return dest;
}

void*
memset(void *s, int c, size_t n) {
    char val = (char)c;
    char *data = s;

    for(size_t i = 0; i < n; ++i) {
        data[i] = val;
    }

    return s;
}

size_t
strlen(const char *string) {
    size_t len = 0;

    while(string[len]) ++len;

    return len;
}

char*
strchr(const char *s, int c) {
    while(*s) {
        if(*s == c) {
            return (char*)s;
        }
    }
    return NULL;
}