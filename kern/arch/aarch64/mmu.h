#ifndef AARCH64_MMU_H
#define AARCH64_MMU_H

#define PAGE_SIZE 4096

#include <kern/types.h>

#define PROT_READ  1
#define PROT_WRITE 2
#define PROT_EXEC  4

/* Allow the user access to the memory */
#define PROT_USER  8

/* Makes the memory nRnEnG on aarch64 */
#define MEM_MMIO   16

struct aarch64_page_table_level3 {
    /** These are the actual virtual->physical mappings on 4K granularity. */
    uint64_t mappings[512];
};

struct aarch64_page_table_level2 {
    /** These are addresses of level3 page tables, plus associated flags. */
    uint64_t mappings[512];
};

struct aarch64_page_table_level1 {
    /** These are addresses of level2 page tables, plus associated flags. */
    uint64_t mappings[512];
};

struct page_table {
    /** These are addresses of level1 page tables, plus associated flags. */
    uint64_t mappings[512];
};

struct address_space; /* Defined by mem/mem.h */

void mmu_init();
void mmu_map(struct page_table *table, uintptr_t virtual_address, uintptr_t physical_address, int flags);

#endif