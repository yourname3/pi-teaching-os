#include <kern/console/console.h>

void load_devices();

/**
 * Machine-independent kernel entry point. Should be called by the boot code.
 */
void
main() {
    load_devices();

    for(int i = 0; i < 20; ++i) { console_putstr("Hello, world!"); }

    for(;;) {}
}