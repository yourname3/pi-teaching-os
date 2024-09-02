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

extern void uart_writeText(const char *buffer);
extern void (*textwrite)(const char*);

void
load_devices() {
    textwrite = uart_writeText;
}