#include "mmu.h"

extern void main(struct physical_memory_map *memory_map);

void
aarch64_boot() {
    physical_address_t start[] = { MAKE_PHYSICAL_ADDRESS(0) };
    // For now: Provide a 1GB area, like on rpi4b. TODO: Maybe read this from the flattened
    // device tree? Should be somewhat doable.
    physical_address_t end[] = { MAKE_PHYSICAL_ADDRESS(0x40000000) };
    struct physical_memory_map map = {
        .start = start,
        .end = end,
        .count = 1
    };

    main(&map);
}