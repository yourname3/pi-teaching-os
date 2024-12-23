#ifndef K_TASK_H
#define K_TASK_H

#include <kern/arch/aarch64/pcb.h>

/**
 * Each task is in one of four states:
 * - Ready: The task is runnable, but not currently running.
 * - Running: The task is currently running on a CPU.
 * - Sleeping: The task is currently asleep.
 * - Zombie: The task is currently a zombie, waiting to be cleaned up.
 */
enum task_state {
    TS_READY,
    TS_RUNNING,
    TS_SLEEPING,
    TS_ZOMBIE,
};

struct task {
    struct pcb pcb;

    /**
     * One important fact about tasks:
     * They are only ever in ONE state at a time.
     * As such, we can keep track of all the tasks in every state by simply
     * keeping one linked list per state, and then having every task exist
     * in exactly ONE of those linked lists at a time.
     * 
     * Therefore, state_next and state_prev keep track of those links.
     */
    struct task *state_next;
    struct task *state_prev;

    enum task_state state;

    int spl;

    char *stack;
};

extern struct task *curtask;

typedef void (*task_entry_fn)(void*);

struct task *task_new();
void task_start(struct task *t, task_entry_fn fn, void *userdata);

void mi_task_startup(task_entry_fn fn, void *userdata);

void task_yield();

void task_bootstrap();

#endif