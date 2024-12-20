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

    console_putc('A');

    struct task *second = task_new(); console_putc('B');
    task_start(second, second_task, NULL); console_putc('C');

    for(;;) {
        console_putc('a');
        task_yield();
    }

   // menu();

    //for(int i = 0; i < 20; ++i) { console_putstr("Hello, world!"); }

    for(;;) {}
}