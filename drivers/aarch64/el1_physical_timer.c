#include "el1_physical_timer.h"

#include <kern/lib.h>

extern uint64_t read_cntfrq(void);

extern void setup_timer_tval(uint64_t val);
extern void setup_timer_irq();

static uint64_t ticks_per_wait;

void
el1_timer_setup() {
    uint64_t freq = read_cntfrq();
    uint64_t desired_ms_per_tick = 20;
    /* milliseconds per timer fire */
    ticks_per_wait = freq * desired_ms_per_tick / 1000;
    printk("el1 timer: freq = %d ticks = %d\n", freq, ticks_per_wait);

    setup_timer_irq();
    setup_timer_tval(ticks_per_wait);
    
}