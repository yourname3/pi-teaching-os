#include <kern/device.h>

#include "power.h"
#include "interrupt_controller.h"
#include "preempt.h"

#include <kern/lib.h>

static void
p_shutdown_dummy() {
    printk("Performing dummy shutdown.");
    /* TODO: Halt other cores */

    /* If we can't do anything else to shutdown, at least spin. */
    for(;;);
}

static void
p_reboot_dummy() {
    panic("dummy reboot unsupported!");
}

DEFINE_DEVICE(power,
    .shutdown = p_shutdown_dummy,
    .reboot = p_reboot_dummy
);

static void
ic_handle_dummy() {

}

DEFINE_DEVICE(intc,
    .handle = ic_handle_dummy
);

DEFINE_DEVICE(preempt);

#define PRINT_DEVICE(devname) \
printk("  %s = %s (%s:%d)\n", #devname "_dev", devname ## _dev->dev_name, devname ## _dev->dev_file, devname ## _dev->dev_line)

void
device_print_all() {
    printk("loaded drivers:\n");
    PRINT_DEVICE(power);
    PRINT_DEVICE(intc);
    PRINT_DEVICE(preempt);
}