#include <kern/task/task.h>

#include <kern/types.h>
#include <kern/mem/mem.h>
#include <kern/lib.h>

#include <arch/pcb.h>
#include <arch/spl.h>

struct task *curtask;

/* Define each of the linked lists for the various tasks. */
static struct task_list tasks_ready = { 0 };
static struct task_list tasks_zombie = { 0 };

static void
task_unlink_from(struct task *task, struct task_list *task_list) {
    assert(task != NULL);

    if(task->list_prev) {
        task->list_prev->list_next = task->list_next;
    }

    if(task->list_next) {
        task->list_next->list_prev = task->list_prev;
    }

    if(task_list) {
        if(task_list->head == task) {
            task_list->head = task->list_next;
        }
        if(task_list->tail == task) {
            task_list->tail = task->list_prev;
        }
    }

    task->list_next = NULL;
    task->list_prev = NULL;
}

/* TODO: Consider using a list_head style interface for the linked list. This would have
 * the following benefits:
 * - No need to keep track of which list a task is in.
 * - The implementation of the various enqueue/dequeue functions is much simpler. */

static void
task_list_move_head(struct task *task, struct task_list *from, struct task_list *to) {
    assert(task != NULL);
    assert(to != NULL);

    task_unlink_from(task, from);
    task->list_next = to->head;
    if(task->list_next) {
        task->list_next->list_prev = task;
    }
    else {
        /* If nothing is in the list, also set the tail. */
        to->tail = task;
    }

    to->head = task;
    assert(task->list_prev == NULL);
}

static void
task_list_move_tail(struct task *task, struct task_list *from, struct task_list *to) {
    assert(task != NULL);
    assert(to != NULL);

    task_unlink_from(task, from);
    task->list_prev = to->tail;
    if(task->list_prev) {
        task->list_prev->list_next = task;
    }
    else {
        to->head = task;
    }

    to->tail = task;
    assert(task->list_next == NULL);
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

    t->list_next = NULL;
    t->list_prev = NULL;
    t->state = TS_READY;

    /* TODO: Set up stack protector. */

    return t;
}

void
task_bootstrap() {
    struct task *kmain = kzalloc(sizeof(*kmain));
    kmain->stack = NULL; /* We already have a stack. */
    kmain->list_next = NULL;
    kmain->list_prev = NULL;
    kmain->state = TS_RUNNING;
    
    //task_list_move_head(kmain, NULL, &tasks_running);
    curtask = kmain;
}

void
task_start(struct task *t, task_entry_fn fn, void *userdata) {
    task_list_move_head(t, NULL, &tasks_ready);
    md_init_new_task_pcb(&t->pcb, (uintptr_t)t->stack, fn, userdata);
}

int nexttask = 0;

struct task*
scheduler() {
    /* Simple round-robin... */
    return tasks_ready.head;
}

struct task_list*
task_state_to_list(enum task_state state) {
    switch(state) {
        case TS_READY: return &tasks_ready;
        case TS_RUNNING: panic("cannot pass TS_RUNNING to task_state_to_list\n");
        case TS_SLEEPING: panic("cannot pass TS_SLEEPING to task_state_to_list\n");
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
    task_list_move_tail(curtask, NULL, task_state_to_list(into_state));

    /* If there is no new task, default to the current. TODO implement idle
     * task. */
    if(!next) next = curtask;

    /* Step 3: Update curtask. We have to do this before the switch because
     * new tasks will not return here and will not otherwise have a chance to 
     * do it. */

    int spl = splhigh();

    struct task *prev = curtask;
    curtask = next;
    /* The current task is in the ready list, due to being obtained from the scheduler. */
    task_unlink_from(curtask, &tasks_ready);

    /* Step 4: Actually perform the context switch */
    md_switch(&prev->pcb, &curtask->pcb);

    /* Note that if we are switching to a new thread, md_switch will not return
     * here. So anything we do here must also be done in mi_task_startup. */

    /* Restore interrupts to whatever state they should be in for the curtask. */
    splx(spl);
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
    /* When we first reach a new task, we need to turn off all interrupts so
     * that it may be preempted. */
    spl0();

    fn(userdata);

    /* ETC */

    task_exit();
}