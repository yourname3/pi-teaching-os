#ifndef K_TASK_H
#define K_TASK_H

#include <kern/arch/aarch64/pcb.h>

struct task {
    struct pcb pcb;

    char *stack;
};

typedef void (*task_entry_fn)(void*);

struct task *task_new();
void task_start(struct task *t, task_entry_fn fn, void *userdata);

void mi_task_startup(task_entry_fn fn, void *userdata);

void task_yield();

void task_bootstrap();

#endif