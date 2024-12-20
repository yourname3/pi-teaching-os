#include <kern/console/console.h>
#include <kern/console/menu.h>

#include <kern/task/task.h>
#include <kern/devices/power.h>
#include <kern/lib.h>

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


    console_putstr("BOOTING_INTO_KERNEL");
    //console_putc('A');

   // struct task *second = task_new(); console_putc('B');
   // task_start(second, second_task, NULL); console_putc('C');

    //for(;;) {
    printk("Hello printk %d %x %p %p", 10, 32, &main, NULL);
        //console_putc('a');
        //task_yield();
        //if(x --< 0) {
        //    the_power->shutdown();
       //}
    //}

   // menu();

    //for(int i = 0; i < 20; ++i) { console_putstr("Hello, world!"); }

    for(;;) {}
}