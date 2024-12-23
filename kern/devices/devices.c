#include <kern/device.h>

#include "power.h"
#include "interrupt_controller.h"
#include "preempt.h"
#include "console.h"

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

static void
p_panic_dummy() {
    printk("dummy panic -- hanging kernel.\n");

    for(;;);
}

DEFINE_DEVICE(power,
    .shutdown = p_shutdown_dummy,
    .reboot = p_reboot_dummy,
    .panic = p_panic_dummy,
);

static void 
ic_handle_dummy() {

}

DEFINE_DEVICE(intc,
    .handle = ic_handle_dummy
);

DEFINE_DEVICE(preempt);

static void
con_putc_dummy(char c) {}
static bool
con_poll_dummy(uint16_t *out) { return false; }
static void
con_clear_dummy(void) {}
static void
con_clear_line_dummy(void) {}
static void
con_cursor_left_dummy(int amount) {}
static void
con_cursor_right_dummy(int amount) {}
static void
con_backspace_dummy(void) {}

DEFINE_DEVICE(console,
    .con_putc = con_putc_dummy,
    .con_poll = con_poll_dummy,
    .con_clear = con_clear_dummy,
    .con_cursor_left = con_cursor_left_dummy,
    .con_cursor_right = con_cursor_right_dummy,
    .con_backspace = con_backspace_dummy
);

#define PRINT_DEVICE(devname) \
printk("  %s = %s (%s:%d)\n", #devname "_dev", devname ## _dev->dev_name, devname ## _dev->dev_file, devname ## _dev->dev_line)

void
device_print_all() {
    printk("loaded drivers:\n");
    PRINT_DEVICE(power);
    PRINT_DEVICE(intc);
    PRINT_DEVICE(preempt);
    PRINT_DEVICE(console);
}