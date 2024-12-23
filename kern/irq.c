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
    /* Compute a reasonable new size for the map. */
    size_t new_alloc = irq_map.allocated;
     if(irq_map.allocated < 8) {
        new_alloc = 8;
    }
    new_alloc *= 2;

    /* First, allocate a new block of entries. */
    struct irq_map_entry *new_entries = kzalloc_or_die(new_alloc * sizeof(*new_entries),
        "failed to allocate new entries for irq_map rehash");

    /* Now, find all the entries in the old map, then hash them in. 
     * This should work as we have more space. */
    for(size_t old_idx = 0; old_idx < irq_map.allocated; ++old_idx) {
        if(irq_map.entries[old_idx].id != IRQ_NULL && irq_map.entries[old_idx].id != IRQ_TOMBSTONE) {
            size_t new_idx = irq_map.entries[old_idx].id % new_alloc;

            for(size_t i = 0; i < new_alloc; ++i) {
                if(new_entries[new_idx].id == IRQ_NULL) {
                    memcpy(&new_entries[new_idx], &irq_map.entries[old_idx], sizeof(struct irq_map_entry));
                    goto found_spot;
                }

                new_idx = (new_idx + 1) % new_alloc;
            }

            /* If we don't find a spot, panic so we know the algorithm is broken.*/
            panic("irq_rehash: failed to rehash entry %zu (IRQ id %zu)", old_idx, irq_map.entries[old_idx].id);

            /* Successfully found a spot */
            found_spot: {}
        }
    }

    /* Now that all the entries are copied over, free the old block and set
     * the irq_map to point to the new data. */
    kfree(irq_map.entries);
    irq_map.entries = new_entries;
    irq_map.allocated = new_alloc;
}

void
irq_register(size_t irq_id, irq_callback_fn fn, void *userdata) {
    assert(fn != NULL);
    assert(irq_id != IRQ_NULL);
    assert(irq_id != IRQ_TOMBSTONE);

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
            irq_map.allocated += 1;
            return;
        }

        idx = (idx + 1) % irq_map.allocated;
    }
    
    panic("irq_register: failed to find an empty hashmap slot");
}

bool
irq_fire(size_t irq_id, int *result) {
    /* Allow for interrupt controllers to pass IRQ_NULL as a value for convenience.
     * Do not allow IRQ_TOMBSTONE. */
    if(irq_id == IRQ_NULL) return false; /* No such interrupt */

    assert(irq_id != IRQ_TOMBSTONE);

    /* Look for the id in the map. */
    size_t idx = irq_id % irq_map.allocated;

    for(size_t i = 0; i < irq_map.allocated; ++i) {
        /* WARNING: DO NOT INDEX ANYTHING WITH i HERE.
         * TODO: Maybe rewrite this logic so this mistake is harder to make? */
        if(irq_map.entries[idx].id == irq_id) {
            int err = irq_map.entries[idx].fn(irq_map.entries[idx].userdata);
            if(result) *result = err;
            return true;
        }

        /* A NULL slot indicates there's no reason to keep searching. */
        if(irq_map.entries[idx].id == IRQ_NULL) {
            return false;
        }

        idx = (idx + 1) % irq_map.allocated;
    }

    /* Never found the matching entry. */
    return false;
}

void
irq_unregister(size_t irq_id) {
    /* Look for the id in the map. */
    size_t idx = irq_id % irq_map.allocated;

    for(size_t i = 0; i < irq_map.allocated; ++i) {
        if(irq_map.entries[i].id == irq_id) {
            /* Clear the entry with a tombstone.
             * TODO: Consider rehashing, but shouldn't really be necessary. */
            irq_map.entries[i].id = IRQ_TOMBSTONE;
            irq_map.entries[i].fn = NULL;
            irq_map.entries[i].userdata = NULL;
            irq_map.allocated -= 1;

            return;
        }

        if(irq_map.entries[i].id == IRQ_NULL) {
            /* We can end the search early. */
            return;
        }

        idx = (idx + 1) % irq_map.allocated;
    }
}