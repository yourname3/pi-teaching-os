/**
 * Although our kernel code is split into machine-independent and
 * machine-dependent portions, there is still one more issue: how do we find
 * devices attached to our machine (such as mice, keyboards, GPUs, timers, etc)?
 * 
 * We could simply write a machine-dependent portion for the kernel for each
 * possible computer. But this would add a lot of extra code to the kernel tree,
 * and there really is no need to have device-specific code in the kernel tree
 * beyond the various different architectures.
 * 
 * Instead, we can provide the drivers for each device separately--either loaded
 * dynamically at runtime, or linked directly in to the kernel.
 * 
 * That still leaves one question: How do we FIGURE OUT which devices we need to
 * load drivers for?
 * 
 * On x86, the CPU provides an "API" of sorts to figure that out, called ACPI.
 * It is difficult to actually implement the code necessary to call that API,
 * but it is doable, and it's also possible to use a common library called
 * ACPICA.
 * 
 * On ARM devices, there is no ACPI. The solution used by Linux is called a
 * "device tree", which is a file provided at boot time to the kernel which tells
 * it a set of drivers that it should load.
 * 
 * Here, we use a similar but in some ways worse solution. We argue that each
 * kind of computer, such as "rpi4b", should get ONE specific-to-itself C file,
 * called devices.c, which provides the function load_devices().
 * 
 * This function will load all the drivers we want to provide to the system.
 */



#include <kern/console/console.h>
#include <drivers/generic/con_vt100.h>

#include <drivers/aarch64/el1_physical_timer.h>
#include <drivers/aarch64/gic_400.h>

#include <drivers/rpi4b/bcm2711_irq.h>

extern void uart_writeByteBlockingActual(char ch, void *unused);
extern bool uart_readByte(char *out, void *unused);

con_vt100 miniuart = {
    .vt100_putc = uart_writeByteBlockingActual,
    .vt100_poll = uart_readByte,
};
console_io_attach miniuart_console = {0};

extern void wdog_init();

void
rpi_timer_init() {
#define TIMER_BASE (((0x40000000 + (0xFE000000 - 0xC0000000)) + 0x00003000))
    volatile uint32_t *control_status = (volatile uint32_t*)TIMER_BASE;
    volatile uint32_t *counter_lo = (volatile uint32_t*)(TIMER_BASE + 4);
    volatile uint32_t *counter_hi = (volatile uint32_t*)(TIMER_BASE + 8);
    // compare[0] = 12
    volatile uint32_t *compare_1 = (volatile uint32_t*)(TIMER_BASE + 16);
    // compare[2] = 20
    volatile uint32_t *compare_3 = (volatile uint32_t*)(TIMER_BASE + 24);

    *compare_1 = (*counter_lo + 1000000);
    *compare_3 = (*counter_lo + 1000000);
}

void
load_devices() {
    create_vt100_console_attachment(&miniuart_console, &miniuart);
    console_attach(&miniuart_console);

    gic_400_init(0x40000000 + (0xFF840000 - 0xC0000000));
    gic_400_assign_irq_cpu(29, 0);
    gic_400_enable(29);

    gic_400_assign_irq_cpu(97, 0);
    gic_400_enable(97);

    gic_400_assign_irq_cpu(99, 0);
    gic_400_enable(99);

    bcm2711_irq_init();
    bcm2711_irq_enable_timer();


    rpi_timer_init();
    

    
    wdog_init();

    //el1_timer_setup();
}