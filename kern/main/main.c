
/* Loaded by devices.c */
void (*textwrite)(const char*);

void load_devices();

/**
 * Machine-independent kernel entry point. Should be called by the boot code.
 */
void
main() {
    load_devices();

    textwrite("Hello, world!");

    for(;;) {}
}