#include <kern/task/task.h>

#include <kern/types.h>
#include <kern/mem/mem.h>

#include <kern/arch/aarch64/pcb.h>

struct task *curtask;

/* TODO: Real task array system... */
struct task *all_tasks[32];
int alltasklen = 0;

struct task*
task_new() {
    struct task *t = kzalloc(sizeof(*t));
    if(!t) {
        return NULL;
    }

    t->stack = kzalloc(STACK_SIZE);
    if(!t->stack) {
        kfree(t);
        return NULL;
    }
    
    /* TODO: Set up stack protector. */

    return t;
}

void
task_bootstrap() {
    struct task *kmain = task_new();
    all_tasks[alltasklen++] = kmain;
    curtask = kmain;
}

void
task_start(struct task *t, task_entry_fn fn, void *userdata) {
    all_tasks[alltasklen++] = t;

    md_init_new_task_pcb(&t->pcb, (uintptr_t)t->stack, fn, userdata);
}

int nexttask = 0;

struct task*
scheduler() {
    /* Simple round-robin... */
    struct task *result = all_tasks[nexttask];
    nexttask = (nexttask + 1) % alltasklen;

    return result;
}

void
mi_switch() {
    /* Step 1: Put the current task in any relevant arrays */

    /* Step 2: Call the scheduler */
    struct task *next = scheduler();

    /* Step 3: Actually perform the context switch */
    md_switch(&curtask->pcb, &next->pcb);

    /* Step 4: Update curtask */
    curtask = next;
}

void
task_yield() {
    mi_switch();
}

void
task_exit() {

}

/**
 * Should be called by a new task when it first starts up. This function is
 * responsible for calling the task_entry_fn so that every task can impicitly
 * call task_exit() at the end.
 */
void
mi_task_startup(task_entry_fn fn, void *userdata) {
    /* todo: DISABLE INTERRUPTS */

    fn(userdata);

    /* ETC */

    task_exit();
}