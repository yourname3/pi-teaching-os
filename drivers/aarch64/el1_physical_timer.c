#include "el1_physical_timer.h"

#include <kern/lib.h>
#include <kern/irq.h>

static uint64_t ticks_per_wait;

static inline void
set_cntp_tval_el0(uint64_t val) {
    __asm__("msr cntp_tval_el0, %0" : : "r" (val));
}

static inline void
set_cntp_ctl_el0(uint64_t val) {
    __asm__("msr cntp_ctl_el0, %0" : : "r" (val));
}

static inline uint64_t
get_cntfrq_el0() {
    uint64_t result;
    __asm__("mrs %0, cntfrq_el0" : "=r" (result));
    return result;
}

int
el1_timer_irq(void *userdata) {
    (void)userdata;

    printk("el1 timer fired\n");

    set_cntp_ctl_el0(1);
    set_cntp_tval_el0(get_cntfrq_el0());

    return 0;
}

void
el1_timer_setup() {
    /* Register an IRQ for the timer firing. */
    irq_register(IRQ_EL1_PHYSICAL_TIMER, el1_timer_irq, NULL);

    uint64_t freq = get_cntfrq_el0();
    uint64_t desired_ms_per_tick = 20;
    /* milliseconds per timer fire */
    ticks_per_wait = freq * desired_ms_per_tick / 1000;
    printk("el1 timer: freq = %d ticks = %d\n", freq, ticks_per_wait);

    set_cntp_ctl_el0(1 << 0);
    set_cntp_tval_el0(freq);
}