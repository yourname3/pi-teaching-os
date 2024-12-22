#include "irq.h"

#include <kern/lib.h>

struct irq_map_entry {
    size_t id;
    irq_callback_fn fn;
    void *userdata;
};

/**
 * We define a basic linear-probing hashmap for IRQs.
 * 
 * This is not a great choice. Interrupt handlers are in general, meant to be
 * extremely fast. Hashmaps are already not the fastest, and linear probing
 * can lead to even more slowdown.
 * 
 * That said, in terms of the actual usage of IRQs in our teaching kernel, there
 * are not that many IRQs and in that case a linear search is pretty fast, and
 * it's arguably made even faster by the fact that we're doing a direct lookup
 * first.
 */
struct {
    struct irq_map_entry *entries;
    size_t allocated;
    size_t used;
} irq_map;

void
irq_init() {
    irq_map.allocated = 8;
    irq_map.used = 0;
    irq_map.entries = kzalloc_or_die(irq_map.allocated * sizeof(*irq_map.entries),
        "failed to allocate irq_map");

    /* All entries are IRQ_NULL right now. */
}

void
irq_rehash() {

}

void
irq_register(size_t irq_id, irq_callback_fn fn, void *userdata) {
    /** Load factor of 2. */
    if((irq_map.used + 1) * 2 >= irq_map.allocated) {
        irq_rehash();
    }

    size_t idx = irq_id % irq_map.allocated;

    /* Just put it into the map. 
     * OPTIMIZATION: If we store the size as a power of two, we can use a bitmask
     * instead of a modulo, which would be a really good idea in an IRQ handler. */
    for(size_t i = 0; i < irq_map.allocated; ++i) {
        if(irq_map.entries[idx].id == IRQ_NULL || irq_map.entries[idx].id == IRQ_TOMBSTONE) {
            irq_map.entries[idx].fn = fn;
            irq_map.entries[idx].id = irq_id;
            irq_map.entries[idx].userdata = userdata;
        }

        idx = (idx + 1) % irq_map.allocated;
    }
    
    panic("irq_register: failed to find an empty hashmap slot");
}

bool
irq_fire(size_t irq_id, int **result) {

}

void
irq_unregister(size_t irq_id) {

}