#include "mmu.h"

#include <kern/lib.h>
#include <kern/mem/mem.h>
#include <kern/lib/linked_list.h>

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

static physical_address_t cache[CACHE_SIZE];
static size_t cache_top = CACHE_SIZE;

static struct physical_memory_map *early_page_table_alloc_src = NULL;
static uint64_t early_page_table_alloc_last = 0;

struct {
    struct frame *next;
    struct frame *prev;
} free_frames; /* Frames that are free to be used */

struct {
    struct frame *next;
    struct frame *prev;
} page_table_frames; /* Frames allocated for use in page tables */

static physical_address_t allocate_page_table() {
    /* If we're early in the physical page allocation, then we can find physical
     * addresses by walking through the values in this array until we see one
     * that is available. */
    if(early_page_table_alloc_src) {
        size_t candidate = early_page_table_alloc_last + 4096;

        size_t i = 0;
        for(;;) {
            if(candidate + PAGE_SIZE - 1 < early_page_table_alloc_src->end[i].val) {
                /* This candidate is good -- use it */
                early_page_table_alloc_last = candidate;
                return MAKE_PHYSICAL_ADDRESS(candidate);
            }

            /* Otherwise, we need to move the candidate to the start of the next
             * physical memory area. */
            i += 1;
            if(i >= early_page_table_alloc_src->count) {
                panic("aarch64 mmu: could not allocate early physical frames");
            }

            candidate = early_page_table_alloc_src->start[i].val;
        }
    }

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

extern char _physmap_pagetable_level1;
extern char _end;

struct phys_area {
    physical_address_t start;
    physical_address_t end;
    struct frame *map;
    size_t map_size;
};

struct phys_area *mem_map = NULL;
size_t mem_map_size = 0;

static void
init_k_map(pagetable_t k_page_table, void *start_ptr, void *end_ptr, int flags) {
    uintptr_t start = (uintptr_t)start_ptr;
    assert(start % 4096 == 0);
    uintptr_t end = (uintptr_t)end_ptr;

    size_t page_count = ((end + PAGE_SIZE - 1) - start) / PAGE_SIZE;
    for(size_t i = 0; i < page_count; ++i) {
        size_t page_addr = start + i * 4096;
        /* The way we've set up the kernel image, the physical address of each kernel page is
         * simply its virtual address minus the 0xFFFF000000000000 value. */
        mmu_map(k_page_table, MAKE_VIRTUAL_ADDRESS(page_addr), MAKE_PHYSICAL_ADDRESS(page_addr - 0xFFFF000000000000), flags);
    }
}

#define KERNEL_PHYSICAL_BASE 0xFFFF800000000000
static uint64_t*
logical_map(uint64_t ptr) {
    uintptr_t virt_address = KERNEL_PHYSICAL_BASE + ptr;
    return (uint64_t*)virt_address;
}

static void
walk_pages(pagetable_t page_table, void (*callback)(physical_address_t phys_addr)) {
    for(size_t i = 0; i < 256; ++i) { // Outermost loop: Walk over 512 GB regions
        uint64_t p0 = *logical_map(page_table.val + i * 8);
        if(p0 & 1) {
            for(size_t j = 0; j < 512; ++j) { // Walk over 1GB regions
                uint64_t p1 = *logical_map((p0 & ~0xfff) + j * 8);
                if(p1 & 1) {
                    for(size_t k = 0; k < 512; ++k) { // Walk over 2MB regions
                        uint64_t p2 = *logical_map((p1 & ~0xfff) + k * 8);
                        if(p2 & 1) {
                            for(size_t l = 0; l < 512; ++l) { // Walk over 4K regions
                                uint64_t p3 = *logical_map((p2 & ~0xfff) + l * 8);
                                if(p3 & 1) {
                                    // If it's valid, then it's an address. Mask off everything else.
                                    uint64_t addr = p3 & ~(0xfff | UXN | PXN);
                                    callback(MAKE_PHYSICAL_ADDRESS(addr));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

struct frame*
physaddr_to_frame(physical_address_t phys_addr) {
    for(size_t i = 0; i < mem_map_size; ++i) {
        if(phys_addr.val >= mem_map[i].start.val && phys_addr.val < mem_map[i].end.val) {
            // No need to check inner size.
            return &mem_map[i].map[phys_addr.val >> 12];
        }
    }
    return NULL;
}

static void
init_own_page(physical_address_t used) {
    struct frame *used_frame = physaddr_to_frame(used);
    ll_unlink(used_frame);
    ll_insert(&page_table_frames, used_frame);
}

static void
init_mem_map(pagetable_t k_page_table, struct physical_memory_map *src) {
    size_t frame_count = 0;
    size_t area_count = src->count;

    for(size_t i = 0; i < src->count; ++i) {
        size_t page_count = (src->end[i].val - src->start[i].val + PAGE_SIZE - 1) / PAGE_SIZE;
        frame_count += page_count;
    }

    uintptr_t mem_map_start = (uintptr_t)&_end;
    assert(mem_map_start % PAGE_SIZE == 0);
    uintptr_t frame_array_start = ((mem_map_start + area_count * sizeof(struct phys_area) + 16 - 1) / 16) * 16;

    uintptr_t alloc_end = ((frame_array_start + frame_count * sizeof(struct frame) + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

    /* Map the memory we want into our address space by allocating it one page at a time. */
    for(uintptr_t ptr = mem_map_start; ptr < alloc_end; ptr += PAGE_SIZE) {
        physical_address_t phys_page = allocate_page_table();
        mmu_map(k_page_table, MAKE_VIRTUAL_ADDRESS(ptr), phys_page, PROT_READ | PROT_WRITE);
    }

    asm volatile (
        "\n\tdsb ishst"
        "\n\ttlbi vmalle1"
        "\n\tdsb ish"
        : : : "memory"
    );

    mem_map = (struct phys_area*)mem_map_start;
    mem_map_size = area_count;

    struct frame *all_frames = (struct frame*)frame_array_start;

    size_t frame_idx = 0;

    ll_init(&free_frames);

    for(size_t i = 0; i < src->count; ++i) {
        size_t page_count = (src->end[i].val - src->start[i].val + PAGE_SIZE - 1) / PAGE_SIZE;

        mem_map[i].start = src->start[i];
        mem_map[i].end   = src->end[i];

        mem_map[i].map = all_frames + frame_idx;
        mem_map[i].map_size = page_count;

        for(size_t j = 0; j < page_count; ++j) {
            mem_map[i].map[j].next = NULL;
            mem_map[i].map[j].prev = NULL;
            mem_map[i].map[j].physical_address = mem_map[i].start.val + j * PAGE_SIZE;
            mem_map[i].map[j].range = NULL;
            mem_map[i].map[j].file = NULL;

            ll_insert(&free_frames, &mem_map[i].map[j]);
        }
    }

    walk_pages(k_page_table, &init_own_page);
}

void
mmu_init(struct physical_memory_map *memory_map) {
    /* In order to allocate page tables early, we start by trying physical pages placed
     * after the end of the kernel. It is possible there is some physical memory before
     * the kernel, but there should almost certainly be some afterwards. */
    early_page_table_alloc_last = (uint64_t)&_end - PAGE_SIZE - 0xffff000000000000;
    early_page_table_alloc_src = memory_map;

    pagetable_t k_page_table = allocate_page_table();

    /* Map the kernel code as read-only, executable */
    init_k_map(k_page_table, &kern_text_start, &kern_text_end, PROT_EXEC | PROT_READ);

    /* Map the kernel read-only data as read-only (should make the code safer) */
    init_k_map(k_page_table, &kern_rodata_start, &kern_rodata_end, PROT_READ);

    /* Map everything else as read-write (but not executable). */
    init_k_map(k_page_table, &kern_data_start, &kern_data_end, PROT_READ | PROT_WRITE);

    for(size_t i = 0; i < 256; ++i) {
        /* Map the logical map onto the latter 256 entries */
        uint64_t physmap_ptr = (uint64_t)&_physmap_pagetable_level1;
        uint64_t *ptr = logical_map(k_page_table.val + (256 + i) * 8);
        *ptr = (physmap_ptr + 4096 * i) | 3;
    }

    /* Install the new map */
    asm volatile (
        "\n\tmsr ttbr1_el1, %0"
        "\n\tdsb ishst" // armv8-a address translation" page 28
        "\n\ttlbi vmalle1" // notes... tlbi alle1 does not work, vmalle1 does work... (but seems to be hypervisor related)
                           // TODO: Is this correct?
        "\n\tdsb ish"
        "\n\tisb" 
        : : "r" (k_page_table.val) : "memory"
    );

    init_mem_map(k_page_table, memory_map);
}

uint64_t*
walk(physical_address_t table, uintptr_t addr, size_t shift) {
    size_t idx = (addr >> shift) & 0x1FF;
    /* The virtual address of this physical address inside our address space. */
    return logical_map(table.val + (idx * sizeof(uint64_t)));
}

physical_address_t
walk_or_create(physical_address_t table_address, uintptr_t addr, size_t shift) {
    uint64_t *mapping = walk(table_address, addr, shift);
    if((*mapping & 1) == 0) {
        /* Unmapped - create new page table */
        physical_address_t result = allocate_page_table();
        *mapping = ((uint64_t)result.val | 3); /* Bit pattern for page table pointer is (address | 0b11) */
        return result;
    }

    /* Otherwise, we have a page table */
    assert((*mapping & 3) == 3);
    /* Clear the last two bits of the mapping, as they aren't actually part of the address. */
    return MAKE_PHYSICAL_ADDRESS(*mapping & ~3);
}

void
mmu_map(pagetable_t table, virtual_address_t virtual_address, physical_address_t physical_address, int flags) {
    /* Find the entry in the page table that we need to map */
    physical_address_t level0 = table;
    /* Hmm... these pointers need to be physical addresses... */
    physical_address_t level1 = walk_or_create(level0, virtual_address.val, 39);
    physical_address_t level2 = walk_or_create(level1, virtual_address.val, 30);
    physical_address_t level3 = walk_or_create(level2, virtual_address.val, 21);
    uint64_t *pte             = walk          (level3, virtual_address.val, 12);

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
    *pte = physical_address.val | pte_flags | AF | SH_INNER_SHAREABLE | 3;
}