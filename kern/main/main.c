#include <kern/console/menu.h>

#include <kern/task/task.h>
#include <kern/devices/power.h>
#include <kern/lib.h>
#include <kern/irq.h>

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
    /* Must initialize IRQ subsystem before we load devices. This is because
     * the devices will almost certainly register some IRQs. */
    irq_init();

    load_devices();
    
    task_bootstrap();

    printk("Welcome to the kernel.\n");
    printk("kern> ");

    //int *bad = (int*)0xF00DF00D;
    //*bad = 20;

    #define TIMER_BASE (((0x40000000 + (0xFE000000 - 0xC0000000)) + 0x00003000))
    volatile uint32_t *control_status = (volatile uint32_t*)TIMER_BASE;
    
    volatile uint32_t *counter_lo = (volatile uint32_t*)(TIMER_BASE + 4);
    volatile uint32_t *counter_hi = (volatile uint32_t*)(TIMER_BASE + 8);
    // compare[0] = 12
    volatile uint32_t *compare_1 = (volatile uint32_t*)(TIMER_BASE + 16);
    // compare[2] = 20
    volatile uint32_t *compare_3 = (volatile uint32_t*)(TIMER_BASE + 24);

    for(;;) {
        //printk("counter = %d compare = %d\n", *counter_lo, *compare_1);
        if(*counter_lo > *compare_1) {
            printk("timer expired....\n");
            break;
        }
        //if(*counter_lo > *compare_1) {
        //    printk("timer expired... wheres my interrupt...\n");
        //}
    }

    for(;;) {}
}