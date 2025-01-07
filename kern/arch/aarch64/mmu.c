#include "mmu.h"

#include <kern/lib.h>

#define CACHE_SIZE 64

#define UXN (1LLU << 54) /* Unprivileged Execute Never */
#define PXN (1LLU << 53) /* Privileged Execute Never */
#define AF  (1LLU << 10) /* Access flag: If this is 0, then access to the memory will fault, allowing us to e.g. swap */
#define SH_NON_SHAREABLE (0 << 8)
#define SH_OUTER_SHAREABLE (2 << 8)
#define SH_INNER_SHAREABLE (3 << 8)
#define AP_K_RW (0 << 6) /* Kernel read-write, user none */
#define AP_U_RW (1 << 6) /* Kernel and user read-write */
#define AP_K_R  (2 << 6) /* Kernel read-only, user none */
#define AP_U_R  (3 << 6) /* Kernel and user read-only */
#define NS (1 << 5)

#define MAIR_IDX0 (0 << 2)
#define MAIR_IDX1 (1 << 2)
#define MAIR_IDX2 (2 << 2)
#define MAIR_IDX3 (3 << 2)
#define MAIR_IDX4 (4 << 2)
#define MAIR_IDX5 (5 << 2)
#define MAIR_IDX6 (6 << 2)
#define MAIR_IDX7 (7 << 2)

static char init_table_cache[4096 * CACHE_SIZE] __attribute__((aligned(4096)));

static void* cache[CACHE_SIZE];
static size_t cache_top = CACHE_SIZE;

static void *allocate_page_table() {
    if(cache_top == 0) {
        panic("aarch64 mmu: ran out of page tables");
    }

    cache_top -= 1;
    return cache[cache_top];
}

extern char kern_text_start;
extern char kern_text_end;

extern char kern_rodata_start;
extern char kern_rodata_end;

extern char kern_data_start;
extern char kern_data_end;

static void
init_k_map(struct page_table *k_page_table, void *start_ptr, void *end_ptr, int flags) {
    uintptr_t start = (uintptr_t)start_ptr;
    uintptr_t end = (uintptr_t)end_ptr;

    size_t page_count = ((end + PAGE_SIZE - 1) - start) / PAGE_SIZE;
    for(size_t i = 0; i < page_count; ++i) {
        /* The way we've set up the kernel image, the physical address of each kernel page is
         * simply its virtual address minus the 0xFFFF000000000000 value. */
        mmu_map(k_page_table, start, start - 0xFFFF000000000000, flags);
    }
}

void
mmu_init() {
    /* Set up some initial page table objects we can allocate for mapping the kernel. */
    for(size_t i = 0; i < CACHE_SIZE; ++i) {
        cache[i] = init_table_cache + 4096 * i;
    }

    struct page_table *k_page_table = allocate_page_table();

    /* Map the kernel code as read-only, executable */
    init_k_map(k_page_table, &kern_text_start, &kern_text_end, PROT_EXEC | PROT_READ);

    /* Map the kernel read-only data as read-only (should make the code safer) */
    init_k_map(k_page_table, &kern_rodata_start, &kern_rodata_end, PROT_READ);

    /* Map everything else as read-write (but not executable). */
    init_k_map(k_page_table, &kern_data_start, &kern_data_end, PROT_READ | PROT_WRITE);
}

uint64_t*
walk(struct page_table *table, uintptr_t addr, size_t shift) {
    size_t idx = (addr >> shift) & 0x1FF;
    return& table->mappings[idx]; 
}

struct page_table*
walk_or_create(struct page_table* table, uintptr_t addr, size_t shift) {
    uint64_t *mapping = walk(table, addr, shift);
    if((*mapping & 1) == 0) {
        /* Unmapped - create new page table */
        struct page_table *result = allocate_page_table();
        *mapping = ((uint64_t)result | 3); /* Bit pattern for page table pointer is (address | 0b11) */
        return result;
    }

    /* Otherwise, we have a page table */
    assert((*mapping & 3) == 3);
    return (struct page_table*)(*mapping);
}

void
mmu_map(struct page_table *table, uintptr_t virtual_address, uintptr_t physical_address, int flags) {
    /* Find the entry in the page table that we need to map */
    struct page_table *level0 = table;
    /* Hmm... these pointers need to be physical addresses... */
    struct page_table *level1 = walk_or_create(level0, virtual_address, 39);
    struct page_table *level2 = walk_or_create(level1, virtual_address, 30);
    struct page_table *level3 = walk_or_create(level2, virtual_address, 21);
    uint64_t *pte             = walk          (level3, virtual_address, 12);

    /* Create the page flags. */
    uint64_t pte_flags = 0;
    bool kernel_only = !(flags & PROT_USER);
    if(flags & PROT_WRITE) {
        /* Note: write implies read due to no write-only pages on aarch64. */
        pte_flags |= (kernel_only ? AP_K_RW : AP_U_RW);
    }
    else if(flags & PROT_READ) {
        /* Memory is read-only (PROT_WRITE not set) */
        pte_flags |= (kernel_only ? AP_K_R : AP_U_R);
    }

    /* Check executable */
    if(!(flags & PROT_EXEC)) {
        /* Make sure memory is not executable unless necessary. */
        pte_flags |= UXN;
        pte_flags |= PXN;
    }
    if(kernel_only) {
        pte_flags |= UXN;
    }

    if(flags & MEM_MMIO) {
        /* If the memory is used for memory mapped I/O, use the 1 MAIR index,
         * otherwise use the 0 index */
        pte_flags |= MAIR_IDX1;
    }
    else {
        pte_flags |= MAIR_IDX0;
    }

    /* Always set the AF flag, otherwise we fault; always set as inner shareable. */
    *pte = pte_flags | AF | SH_INNER_SHAREABLE;
}