#include <kern/task/task.h>

#include <kern/types.h>
#include <kern/mem/mem.h>
#include <kern/lib.h>

#include <kern/arch/aarch64/pcb.h>

struct task *curtask;

/* Define each of the linked lists for the various tasks. */
static struct task *tasks_ready = NULL;
static struct task *tasks_running = NULL;
static struct task *tasks_sleeping = NULL;
static struct task *tasks_zombie = NULL;

static void
task_unlink_state(struct task *task) {
    if(task->state_prev) {
        task->state_prev->state_next = task->state_next;
    }

    if(task->state_next) {
        task->state_next->state_prev = task->state_prev;
    }

    task->state_next = NULL;
    task->state_prev = NULL;
}

static void
task_insert(struct task *task, struct task **list) {
    task_unlink_state(task);
    task->state_next = *list;
    if(task->state_next) {
        task->state_next->state_prev = task;
    }

    *list = task;
    task->state_prev = NULL;
}

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
    struct task *kmain = kzalloc(sizeof(*kmain));
    kmain->stack = NULL; /* We already have a stack. */
    
    task_insert(kmain, &tasks_running);
}

void
task_start(struct task *t, task_entry_fn fn, void *userdata) {
    task_insert(t, &tasks_ready);
    md_init_new_task_pcb(&t->pcb, (uintptr_t)t->stack, fn, userdata);
}

int nexttask = 0;

struct task*
scheduler() {
    /* Simple round-robin... */
    return tasks_ready;
}

struct task**
task_state_to_link(enum task_state state) {
    switch(state) {
        case TS_READY: return &tasks_ready;
        case TS_RUNNING: return &tasks_running;
        case TS_SLEEPING: return &tasks_sleeping;
        case TS_ZOMBIE: return &tasks_zombie;
        default:
            panic("invalid task_state %d\n", state);
    }
}

void
mi_switch(enum task_state into_state) {
    /* Step 1: Call the scheduler. Do this before descheduling the current task
     * for reasons. */
    struct task *next = scheduler();

    /* Step 2: Remove the current task from RUNNING and put it into its new
     * linked list. Note that we do this after scheduler to try to make it
     * possible to implement a cool easy scheduler but TODO: that doesn't
     * actually work */
    assert(into_state != TS_RUNNING);
    task_insert(curtask, task_state_to_link(into_state));

    /* If there is no new task, default to the current. TODO implement idle
     * task. */
    if(!next) next = curtask;

    /* Step 3: Update curtask. We have to do this before the switch because
     * new tasks will not return here and will not otherwise have a chance to 
     * do it. */

    struct task *prev = curtask;
    curtask = next;
    task_unlink_state(curtask);

    /* Step 4: Actually perform the context switch */
    md_switch(&prev->pcb, &curtask->pcb);

    /* Note that if we are switching to a new thread, md_switch will not return
     * here. So anything we do here must also be done in mi_task_startup. */
}

void
task_yield() {
    /* Remain runnable. */
    mi_switch(TS_READY);
}

void
task_exit() {
    /* This task has become a zombie. */
    mi_switch(TS_ZOMBIE);
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