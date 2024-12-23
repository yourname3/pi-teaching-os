#include "preempt.h"

#include <kern/task/task.h>

void
preempt_fire() {
    /* The most important task for the timer in preemptive multitasking:
     * Regularly yield the current thread. */
    task_yield();
}

/**
 * Reports the frequency at which we wish to perform preemptive multitasking.
 * The timer driver should at least semi-regularly call this function to know what
 * rate to fire at.
 */
uint64_t
preempt_goal_hz() {
    /* For simplicity, report a constant value of 100 HZ (timeslice = 10 ms). */
    return 100;
}