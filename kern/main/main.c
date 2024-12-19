#include <kern/console/console.h>
#include <kern/console/menu.h>

#include <kern/task/task.h>

void load_devices();

void second_task(void *userdata) {
    for(;;) {
        console_putc('b');
        task_yield();
    }
}

/**
 * Machine-independent kernel entry point. Should be called by the boot code.
 */
void
main() {
    load_devices();
    
    task_bootstrap();

    struct task *second = task_new();
    task_start(second, second_task, NULL);

    for(;;) {
        console_putc('a');
        task_yield();
    }

   // menu();

    //for(int i = 0; i < 20; ++i) { console_putstr("Hello, world!"); }

    for(;;) {}
}