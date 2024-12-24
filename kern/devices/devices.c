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

#define PRINT_DEVICE(dev) \
printk("  %-24s (%s:%d)\n", \
    dev->name, \
    dev->file, \
    dev->line)

struct dev_header *device_list;

void
device_print_all() {
    printk("loaded drivers:\n");
    struct dev_header *dev = device_list;
    while(dev) {
        PRINT_DEVICE(dev);
        dev = dev->next;
    }
}