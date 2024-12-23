#include "el1_physical_timer.h"

#include <kern/lib.h>
#include <kern/irq.h>

extern uint64_t read_cntfrq(void);

extern void setup_timer_tval(uint64_t val);
extern void setup_timer_irq();

static uint64_t ticks_per_wait;

int
el1_timer_irq(void *userdata) {
    (void)userdata;

    printk("el1 timer fired\n");

    __asm__(
        "mov x0, #((1 << 0) | (0 << 1))\n"
        "msr cntp_ctl_el0, x0\n"
    );

    return 0;
}

void
el1_timer_setup() {
    /* Register an IRQ for the timer firing. */
    irq_register(IRQ_EL1_PHYSICAL_TIMER, el1_timer_irq, NULL);

    uint64_t freq = read_cntfrq();
    uint64_t desired_ms_per_tick = 20;
    /* milliseconds per timer fire */
    ticks_per_wait = freq * desired_ms_per_tick / 1000;
    printk("el1 timer: freq = %d ticks = %d\n", freq, ticks_per_wait);

    setup_timer_irq();
    setup_timer_tval(ticks_per_wait);
}