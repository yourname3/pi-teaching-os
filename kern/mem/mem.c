#include <kern/mem/mem.h>

/* TODO: Implement real kernel allocator? */
static char bump_heap[16384];
static char *heap_next = bump_heap;

void *kzalloc(size_t size) {
    char *result = heap_next;
    /* Ensure we're always 16 byte aligned */
    heap_next += (size + 15) % 16;
    /* TODO Implement memset */
    for(size_t i = 0; i < size; ++i) {
        result[i] = 0;
    }
    return result;
}
void kfree(void *mem) {
    /* TODO */
}