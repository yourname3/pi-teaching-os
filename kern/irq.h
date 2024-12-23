#ifndef K_IRQ_H
#define K_IRQ_H

/**
 * We define a generic IRQ interface as follows:
 * 
 * 1. Each IRQ has a unique integer identifier. This allows the same IRQ to be
 *    transparently handled by multiple possible interrupt controllers.
 * 
 * 2. The interrupt controller driver is responsible for the IRQ lifecycle:
 *    it must figure out which (if any) kernel-level IRQ callbacks to call, and
 *    then acknowledge the IRQ to the hardware interrupt controller if needed.
 * 
 *    However, because the hardware associated with an IRQ may also need to be
 *    reset before that IRQ is handled, the sequence of events is as follows:
 *    1. The architecture-specific exception handling code is fired, and 
 *       calls the intc_dev->handle() function.
 *    2. The intc_dev->handle() function calls any registered IRQ callbacks.
 *    3. The intc_dev->handle() performs interrupt acknowledgement.
 *    4. Everything is finished; each function returns back up the call stack
 *       and eventually some kind of "interrupt return" assembly instruction is
 *       called.
 */


#include <kern/types.h>

typedef int (*irq_callback_fn)(void *userdata);

/* Include the device-specific list of IRQ numbers. */
#include <device/irqs.h>

_Static_assert(IRQ_NULL == 0, "IRQ_NULL must exist and have value 0.");
_Static_assert(IRQ_TOMBSTONE == 1, "IRQ_TOMBSTONE must exist and have value 1.");

void irq_init();
void irq_register(size_t irq_id, irq_callback_fn fn, void *userdata);
bool irq_fire(size_t irq_id, int *result);
void irq_unregister(size_t irq_id);

enum irq_action {
    IACT_NONE = 0,
    IACT_PREEMPT,
};

#endif