#ifndef K_DEVICES_PREEMPT_H
#define K_DEVICES_PREEMPT_H

#include <kern/lib.h>

struct preempt_device {
    const char *dev_name;
    const char *dev_file;
    uint64_t    dev_line;
};

/**
 * A function hook that should be regularly called by whatever timer we are
 * currently using to perform preemptive multitasking.
 */
void preempt_fire();

/**
 * Reports the frequency at which we wish to perform preemptive multitasking.
 * The timer driver should at least semi-regularly call this function to know what
 * rate to fire at.
 */
uint64_t preempt_goal_hz();

#endif