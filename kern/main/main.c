#include <kern/console/menu.h>

#include <kern/task/task.h>
#include <kern/devices/power.h>
#include <kern/lib.h>

void load_devices();

void second_task(void *userdata) {
    for(;;) {
        printk("second task");
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

    printk("Welcome to the kernel.\n");
    printk("kern> ");

    for(;;) {}
}