#include <kern/console/console.h>

void load_devices();

/**
 * Machine-independent kernel entry point. Should be called by the boot code.
 */
void
main() {
    load_devices();

    console_putstr("Hello, world!");

    for(;;) {}
}