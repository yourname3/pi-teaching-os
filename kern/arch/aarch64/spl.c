#include "spl.h"

#include <kern/task/task.h>

/* Spl is initialized at 0 (all interrupts on) as that's what happens in boot.S. */
int curspl = 0;

int
splhigh() {
    int result = curspl;

    /* Disable all interrupts */
    __asm__ volatile(
        "msr daifset, #0xF\n"
    );

    /* curspl = 1. We do this after disabling interrupts, because if we do
     * it before, there's a chance we're interrupted right after setting 
     * curspl and before we actually turn off interrupts.
     * 
     * But if we wait until after turning off the interrupts, then even though
     * we may have been interrupted right up until the daifset instruction,
     * we will be safe once we actually get here. */
    curspl = 1;

    curtask->spl = 1;

    return result;
}

int
spl0() {
    int result = curspl;

    curtask->spl = 0;

    /* Here we update curspl before actually turning on interrupts.
     *
     * The logic:
     * - If the interrupts are already off, it doesn't matter when we set
     *   curspl to 0 because it's already 0.
     * - If the interrupts are on, then setting it to 0 before disabling them
     *   guarantees it actually reflects the state when it matters (i.e.
     *   when interrupts are possible). */
    curspl = 0;

    __asm__ volatile(
        "msr daifclr, #0xF\n"
    );

    return result;
}

int
splx(int spl) {
    if(spl > 0) {
        return splhigh();
    }
    return spl0();
}
