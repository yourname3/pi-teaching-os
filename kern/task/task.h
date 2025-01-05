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
     * Therefore, list_next and list_prev keep track of those links.
     * 
     * When a task is sleeping, it is simply placed onto a separate list,
     * passed to task_sleep and task_wakeall.
     */
    struct task *list_next;
    struct task *list_prev;

    enum task_state state;

    int spl;

    char *stack;
};

struct task_list {
    struct task *head;
    struct task *tail;
};

/* TODO: Implement spinlocks */
struct spinlock;

extern struct task *curtask;

typedef void (*task_entry_fn)(void*);

struct task *task_new();
void task_start(struct task *t, task_entry_fn fn, void *userdata);

void task_sleep(struct task *task, struct task_list *sleep_list, struct spinlock *spinlock);
void task_wakeall(struct task_list *sleep_list);

void mi_task_startup(task_entry_fn fn, void *userdata);

void task_yield();

void task_bootstrap();

#endif