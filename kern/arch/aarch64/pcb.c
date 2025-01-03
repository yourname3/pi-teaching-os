#include "pcb.h"

#include <kern/lib.h>
#include <kern/task/task.h>

void
md_init_new_task_pcb(
    struct pcb *pcb,
    uintptr_t stack,
    void (*entry_fn)(void*),
    void *userdata) {
    /* The main idea of the PCB setup is that we need to do two things:
     * 1. Set up the stack pointer to actually point to the new stack
     * 2. Set up the values on the stack so that aarch64_switch will correctly
     *    pass them to mi_task_startup. */

    /* 'stack' points to the start of a chunk of memory. Because the stack
     * grows downwards, we need to setup SP to point at the end. */

    pcb->sp = stack + STACK_SIZE;

    /* aarch64_switch is expecting 256 bytes of space on the stack holding
     * the 'old' state. As such, allocate those bytes here. */

    pcb->sp -= 256;

    /* We need to do the following:
     * - Put mi_task_startup in lr (x30) (the return address) 
     * - Put the function pointer fn in x0
     * - Put the userdata argument in x1
     * We simply copy the offsets used to do this from the switch.S code. */

    char *stackdata = (char*)pcb->sp;
    void *task_startup = &mi_task_startup;
    memcpy(stackdata + 0xF0, &task_startup, sizeof(uintptr_t));

    memcpy(stackdata + 0x00, &entry_fn, 8);
    memcpy(stackdata + 0x08, &userdata, 8);
}

extern void aarch64_switch(struct pcb *, struct pcb *);

void
md_switch(struct pcb *old_pcb, struct pcb *new_pcb) {
    aarch64_switch(old_pcb, new_pcb);
}