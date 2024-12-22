#include <kern/mem/mem.h>

#include <kern/lib.h>

/* TODO: Implement real kernel allocator? */
static char bump_heap[16384];
static char *heap_next = bump_heap;

void *kzalloc(size_t size) {
    char *result = heap_next;
    /* Ensure we're always 16 byte aligned */
    heap_next += size + ((16 - size) % 16);

    memset(result, 0, size);
    return result;
}

void *kzrealloc(void *ptr, size_t new_size) {
    void *new_alloc = kzalloc(new_size);
    if(new_alloc) {
        kfree(ptr);
        return new_alloc;
    }
    return NULL;
}

void *kzalloc_or_die(size_t size, const char *fmt, ...) {
    void *result = kzalloc(size);
    if(!result) {
        /* TODO vprintk / vpanic */
        panic(fmt);
    }
    return result;
}

void *kzrealloc_or_die(void *ptr, size_t new_size, const char *fmt, ...) {
    void *result = kzrealloc(ptr, new_size);
    if(!result) {
        panic(fmt);
    }
    return result;
}

void kfree(void *mem) {
    /* TODO */
}