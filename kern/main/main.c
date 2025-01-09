#include <kern/console/menu.h>

#include <kern/task/task.h>
#include <kern/devices/power.h>
#include <kern/lib.h>
#include <kern/irq.h>
#include <kern/device.h>
#include <kern/mem/mem.h>

void load_devices();

void second_task(void *userdata) {
    for(;;) {
        printk("B");
    }
}

void third_task(void *userdata) {
    for(;;) {
        printk("C");
    }
}

extern char kern_text_start;
extern char kern_text_end;

/**
 * Machine-independent kernel entry point. Should be called by the boot code.
 */
void
main() {
    //mmu_init();

    /* Must initialize IRQ subsystem before we load devices. This is because
     * the devices will almost certainly register some IRQs. */
    irq_init();
    
    task_bootstrap();

    load_devices();

    printk("Welcome to the kernel. text = %p - %p\n", &kern_text_start, &kern_text_end);
    device_print_all();
    printk("kern> \n");

    //struct task *second = task_new();
    //task_start(second, second_task, NULL);

    //struct task *third = task_new();
   // task_start(third, third_task, NULL);

    for(;;) {
       // printk("A");
    }
}