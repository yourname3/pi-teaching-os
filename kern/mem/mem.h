#ifndef K_MEM_H
#define K_MEM_H

#include <kern/types.h>
#include <arch/mmu.h>

struct frame;
struct virtual_address_range;
struct address_space;
struct file;

struct frame {
    struct frame *next;
    struct frame *prev;

    uintptr_t physical_address;

    struct virtual_address_range *range;
    struct file *file;
};

struct virtual_address_range {
    struct virtual_address_range *next;
    struct virtual_address_range *prev;

    uintptr_t start;
    uintptr_t end;

    struct virtual_address_range *parent;
    struct virtual_address_range *child;
};

struct address_space {
    uintptr_t start;
    uintptr_t end;

    struct virtual_address_range *range_list;

    pagetable_t page_table;
};

void *kzalloc(size_t size);
void kfree(void *mem);

#endif