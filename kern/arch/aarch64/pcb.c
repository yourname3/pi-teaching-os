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

    printk("md_init_new_task_pcb:\r\n    entry_fn = %p\r\n    userdata = %p\r\n    task_startup = %p\r\n", entry_fn, userdata, task_startup);
}

extern void aarch64_switch(struct pcb *, struct pcb *);

void
aarch64_halfway(struct pcb *old_pcb, struct pcb *new_pcb, uintptr_t new_sp) {
    printk("MADE IT TO CHECKPOINT 5\r\n");
    printk("old_pcb->sp = %p new_pcb->sp = %p\r\n", old_pcb->sp, new_pcb->sp);
    printk("new_pcb[x0] = %p\r\n", ((uintptr_t*)(new_pcb->sp))[0]);
    printk("new_pcb[x1] = %p\r\n", ((uintptr_t*)(new_pcb->sp))[1]);
    printk("new_pcb[lr] = %p\r\n", ((uintptr_t*)(new_pcb->sp))[30]);
    printk("new_sp = %p\r\n", new_sp);
}

void
md_switch(struct pcb *old_pcb, struct pcb *new_pcb) {
    printk("old_pcb->sp = %p new_pcb->sp = %p\r\n", old_pcb->sp, new_pcb->sp);
    printk("new_pcb[x0] = %p\r\n", ((uintptr_t*)(new_pcb->sp))[0]);
    printk("new_pcb[x1] = %p\r\n", ((uintptr_t*)(new_pcb->sp))[1]);
    printk("new_pcb[lr] = %p\r\n", ((uintptr_t*)(new_pcb->sp))[30]);
    aarch64_switch(old_pcb, new_pcb);
}