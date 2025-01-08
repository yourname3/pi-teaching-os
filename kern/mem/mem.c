#include <kern/mem/mem.h>

#include <kern/lib.h>

/**
 * More of a sketch of the memory system:
 * - struct frame: contains information about a physical page, e.g.
 *   is it allocated, how many things reference it, who owns (?) it
 *   Note: In order to e.g. swap a page out, we would have to invalidate
 *   that page in EVERY page table that references it. Storing only one
 *   guy is not enough. Maybe we could do something like have each
 *   virtual_address_range keep track of COW'd virtual_address_ranges?
 *   In that case, a page invalidation would be able to locate other
 *   ranges that might also need invalidating.
 * 
 * - struct virtual_address_range: keeps track "only" of how a virtual address
 *   space is divied up. This doesn't track any information about how this
 *   address range is mapped to physical pages.
 *
 * - struct page_table: architecture-specific page table. Usually just refers
 *   to a literal page table on that architecture. A architecture-specific API
 *   is defined that lets us do things like map the page, set it to read-only,
 *   etc.
 *
 *   Note that there would be one page_table for userspace and one for kernel
 *   space. This is pretty nice on aarch64 but would likely look a little more
 *   ugly on x86.
 */

struct frame*
allocate_frame(struct file *mapped) {
    return NULL;
}

struct virtual_address_range*
new_range(struct address_space *space, void *desired_ptr, size_t length) {
    return NULL;
}

uintptr_t
kmmap(struct address_space *space, void *desired_ptr, size_t length, int prot, int flags, struct file *file, size_t offset) {
    /**
     * kmmap: core memory mapping API for the kernel?
     * Lets us:
     * - allocate virtual memory
     * - map specific virtual addresses
     * - map physical MMIO addresses into our address space
     * - handle file mmap'ing
     * - implement the mmap syscall? */
    size_t page_count = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    
    struct virtual_address_range *range = new_range(space, desired_ptr, length);

    for(size_t i = 0; i < page_count; ++i) {
        struct frame *frame = allocate_frame(file);
        mmu_map(space->page_table, MAKE_VIRTUAL_ADDRESS(range->start + i * PAGE_SIZE), MAKE_PHYSICAL_ADDRESS(frame->physical_address), flags);
    }

    return range->start;
}

/* TODO: Implement real kernel allocator? */
/* The initial heap must be aligned to 16 bytes for our other alignment logic to work. */
static char bump_heap[16384] __attribute__((aligned (16)));
static char *heap_next = bump_heap;

void *kzalloc(size_t size) {
    char *result = heap_next;
    /* Ensure we're always 16 byte aligned */
    heap_next += size + ((16 - size) % 16);

    memset(result, 0, size);
    return result;
}

void *kzrealloc(void *ptr, size_t new_size) {
    panic("kzrealloc doesn't function correctly yet.");

    void *new_alloc = kzalloc(new_size);
    if(new_alloc) {
        // TODO: We need to copy over the old size, but this requires we know
        // the old size. We need to implement a real allocator.
        // memcpy(new_alloc, ptr, )
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