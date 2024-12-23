#include "power.h"
#include "interrupt_controller.h"

#include <kern/lib.h>
#include <kern/console/console.h>

void p_shutdown_dummy() {
    console_putstr("Performing dummy shutdown.");
    /* TODO: Halt other cores */

    /* If we can't do anything else to shutdown, at least spin. */
    for(;;);
}

void p_reboot_dummy() {
    panic("dummy reboot unsupported!");
}

struct power_attach power_dummy = {
    .shutdown = p_shutdown_dummy,
    .reboot = p_reboot_dummy
};

struct power_attach *the_power = &power_dummy;

static void
ic_handle_dummy() {

}

struct intc_device intc_dummy = {
    .handle = ic_handle_dummy
};

struct intc_device *intc_dev = &intc_dummy;