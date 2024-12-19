#ifndef AARCH64_PCB_H
#define AARCH64_PCB_H

#include <kern/types.h>

/* Size of the kernel stack for each task, in bytes. */
#define STACK_SIZE 4096

struct pcb {
    /* Keep track of the stack pointer when we switch tasks. */
    uint64_t sp;
};

void md_init_new_task_pcb(struct pcb *pcb, uintptr_t stack, void (*entry_fn)(void*), void *userdata);
void md_switch(struct pcb *old_pcb, struct pcb *new_pcb);

#endif