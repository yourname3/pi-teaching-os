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

typedef struct virtual_address_t {
    uintptr_t val;
} virtual_address_t;

typedef struct physical_address_t {
    uintptr_t val;
} physical_address_t;

#define MAKE_VIRTUAL_ADDRESS(value) ((struct virtual_address_t){ .val = (uintptr_t)value })
#define MAKE_PHYSICAL_ADDRESS(value) ((struct physical_address_t){ .val = (uintptr_t)value })

struct page_table {
    /** These are addresses of level1 page tables, plus associated flags. */
    physical_address_t mappings[512];
};

/** 
 * On aarch64, the pagetable_t type is simply a pointer to the level 0 page table.
 * Because such a pointer must be a physical address, use the relevant typedef.
 */
typedef physical_address_t pagetable_t;

struct address_space; /* Defined by mem/mem.h */

struct physical_memory_map {
    physical_address_t *start;
    physical_address_t *end;
    size_t count;
};

void mmu_init(struct physical_memory_map *memory_map);
void mmu_map(pagetable_t table, virtual_address_t virtual_address, physical_address_t physical_address, int flags);

#endif