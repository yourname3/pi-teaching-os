#include <kern/console/menu.h>

#include <kern/task/task.h>
#include <kern/devices/power.h>
#include <kern/lib.h>
#include <kern/irq.h>
#include <kern/device.h>

void load_devices();

void second_task(void *userdata) {
    for(;;) {
        printk("second task\n");
    }
}

/**
 * Machine-independent kernel entry point. Should be called by the boot code.
 */
void
main() {
    /* Must initialize IRQ subsystem before we load devices. This is because
     * the devices will almost certainly register some IRQs. */
    irq_init();
    
    task_bootstrap();

    //struct task *second = task_new();
    //task_start(second, second_task, NULL);

    load_devices();

    printk("Welcome to the kernel.\n");
    device_print_all();
    printk("kern> ");

    printk("abcd abcd abcd abcd abcd abcd 1234 1234 1234 1234 1234 1234 1234 1234 1234 1234 1234 %d\n", 5000);

    for(;;) {
        //printk("first task\n");
    }
}