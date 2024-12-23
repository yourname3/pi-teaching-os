#include <kern/lib.h>

#include <arch/spl.h>
#include <kern/devices/power.h>

void
panic(const char *fmt, ...) {

    /**
     * In case we end up recursively panicking, we don't want to repeatedly
     * perform the same steps in the panic. We manually implement a "step"
     * counter to ensure that panic() does not repeat itself.
     */
    static volatile int step = 0;

    if(step == 0) {
        /* Turn off interrupts. */
        splhigh();
        step = 1;
    }

    if(step == 1) {
        /* TODO: Kill other threads and halt all CPUs. */
        step = 2;
    }

    if(step == 2) {
        /* Actually print the panic message. We both start and end with a 
         * newline to make the message as readable as possible onscreen. */

        printk("\npanic: ");

        va_list va;
        va_start(va, fmt);
        vprintk(fmt, va);
        va_end(va);

        printk("\n");
        step = 3;
    }

    /* TODO: Add steps such as:
     * - Syncing the filesystem
     * - Trying to drop to the debugger 
     * - more...? */

    if(step == 3) {
        /* Device-dependent panic operation. This could halt or try to reboot. */
        power_dev->panic();
        step = 4;
    }

    /* If we get here, just hang. */
    for(;;) { }
}