#ifndef RPI4B_IRQS_H
#define RPI4B_IRQS_H

/**
 * NOTE: This is not necessarily the final design of this for the kernel.
 * 
 * We need, somewhere, to actually define all the IRQ numbers used by our device.
 * The kernel could in theory just always use all the IRQ numbers, and that would
 * be the best in terms of ability to use the same compiled kernel in a cross-platform
 * manner.
 * 
 * But we don't really need a system that sophisticated, and it's not too bad to
 * just bake the IRQ numbers we need into the kernel. So we have each "device"
 * provide its own irqs.h, and then just define all the IRQs there.
 * 
 * IMPORTANT: These IRQ numbers are NOT the same as the on-chip IRQ numbers. These
 * identifiers are merely used by the kernel to uniquely refer to each IRQ. The
 * interrupt controller device driver has to then map these IRQs to on-chip
 * numbers.
 */

enum {
    /** All devices are required to define IRQ_NULL to be 0. */
    IRQ_NULL = 0,
    /** All devices are required to define IRQ_TOMBSTONE to be 1. */
    IRQ_TOMBSTONE = 1,
    IRQ_EL1_PHYSICAL_TIMER = 2,
};

#endif