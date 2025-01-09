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

extern char kern_init_pagetable_cache;

static physical_address_t cache[CACHE_SIZE];
static size_t cache_top = CACHE_SIZE;

static physical_address_t allocate_page_table() {
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

/*static inline void
load_ttbr1_el1(uint64_t value) {
    asm volatile(
        "\n\tmsr ttbr1_el1, %0"
        "\n\tdsb ishst" // "armv8-a address translation" page 28
        "\n\ttlbi alle1is" // note: apparently invalidating the tlb here doesn't work?
        "\n\tdsb ish"
        "\n\tisb"
        : : "r" (value) : "memory");
}*/

extern void aarch64_load_ttbr1_el1_tlb_trampoline(uint64_t value);

static void
phys_write(uint64_t phys_addr, uint64_t value) {
    uint64_t addr = phys_addr + 0xFFFF800000000000;
    uint64_t *ptr = (uint64_t*)addr;
    *ptr = value;
}

void
mmu_init() {
    /* Set up some initial page table objects we can allocate for mapping the kernel. */
    uintptr_t cache_physaddr = (uintptr_t)&kern_init_pagetable_cache;
    for(size_t i = 0; i < CACHE_SIZE; ++i) {
        cache[i] = MAKE_PHYSICAL_ADDRESS(cache_physaddr + 4096 * i);
    }

    pagetable_t k_page_table = allocate_page_table();

    /* Map the kernel code as read-only, executable */
    init_k_map(k_page_table, &kern_text_start, &kern_text_end, PROT_EXEC | PROT_READ);

    /* Map the kernel read-only data as read-only (should make the code safer) */
    init_k_map(k_page_table, &kern_rodata_start, &kern_rodata_end, PROT_READ);

    /* Map everything else as read-write (but not executable). */
    init_k_map(k_page_table, &kern_data_start, &kern_data_end, PROT_READ | PROT_WRITE);

    // TEST CODE: write a simple page table just like in boot.S
    // This checks whether load_ttbr1_el1 seems to work.
    pagetable_t test_page_table1 = allocate_page_table();
    pagetable_t test_page_table2 = allocate_page_table();
    pagetable_t test_page_table3 = allocate_page_table();
    size_t phys_addr_pte = 0;
    for(size_t i = 0; i < 512; ++i) {
        phys_write(test_page_table3.val + i * 8, AF | SH_INNER_SHAREABLE | MAIR_IDX0 | 1 | phys_addr_pte);
        phys_addr_pte += 4096;
    }
    phys_write(test_page_table2.val,  AF | SH_INNER_SHAREABLE | MAIR_IDX0 | 1); // setting this one up to point to AF | SH_INNER_SHAREABLE | MAIR_IDX0 | 1 seems to work..
    
    phys_write(test_page_table2.val + 8, AF | SH_INNER_SHAREABLE | MAIR_IDX0 | 1); /* NOTE: New debug strat: Try running a level 3 page table that isn't critical to the kernel functioning. */
    phys_write(test_page_table2.val + 16, test_page_table3.val | 3);
    phys_write(test_page_table2.val + 24, AF | SH_INNER_SHAREABLE | MAIR_IDX0 | 3);
    phys_write(test_page_table2.val + 32, test_page_table3.val | 3);

    phys_write(test_page_table1.val, test_page_table2.val | 3);
    //phys_write(test_page_table1.val, AF | SH_INNER_SHAREABLE | MAIR_IDX0 | 1);
    phys_write(k_page_table.val, test_page_table1.val | 3);

    /* Install the new map */
    aarch64_load_ttbr1_el1_tlb_trampoline(k_page_table.val);
}

#define KERNEL_PHYSICAL_BASE 0xFFFF800000000000

uint64_t*
walk(physical_address_t table, uintptr_t addr, size_t shift) {
    size_t idx = (addr >> shift) & 0x1FF;
    /* The virtual address of this physical address inside our address space. */
    uintptr_t virt_address = KERNEL_PHYSICAL_BASE + table.val + (idx * sizeof(uint64_t));
    return (uint64_t*)virt_address;
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
    //    pte_flags |= UXN;
    //    pte_flags |= PXN;
    }
    if(kernel_only) {
    //    pte_flags |= UXN;
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
    *pte = physical_address.val | pte_flags | AF | SH_INNER_SHAREABLE | 1;
}