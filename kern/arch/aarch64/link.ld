SECTIONS
{
    /* Hmm..
     * So we have kind of two things here:
     * 1. We want to have the kernel code referencing things like, the data
     *    at address 0xFFFF0000_00000000 and such.
     * 2. We need to have the bootloader actually load us into physical (?)
     *    memory.
     *
     * It seems like what we really want to do is embed the entire kernel elf
     * into a second, bootloader sort of elf that sets up the page table as
     * necessary and then jumps into the kernel...
     *
     * The other option is to just keep it simple stupid and identity map
     * everything that is so low. But then we don't get to use the ARM 
     * ttbr1 setup. */

    . = 0x80000;

    .text : ALIGN(4K) { KEEP(*(.text.boot)) }

    .data.boot : ALIGN(4K) { KEEP(*(.data.boot)) }

    . += 0xFFFF000000000000;

    /**
     * This somewhat awkward setup is necessary because we need the following
     * relationship:
     * VMA = LMA + exactly 0xFFFF000000000000
     *
     * Trying to get the linker to e.g. allocate using a MEMORY setup will
     * instead result in the VMA being different by however much was already
     * allocated to the physical memory.
     *
     * Instead, simply manually incrementing . (the current VMA) to point
     * exactly where we need it, and then specifying each LMA by subtracting
     * that again, gives the needed results.
     */
    .text.kern : AT(ADDR(.text.kern) - 0xFFFF000000000000) {
        *(.text .text.* .gnu.linkonce.t*)
    }
    .rodata : AT(ADDR(.rodata) - 0xFFFF000000000000) { *(.rodata .rodata.* .gnu.linkonce.r*) }
    PROVIDE(_data = .);
    .data : AT(ADDR(.data) - 0xFFFF000000000000) {
        *(.data .data.* .gnu.linkonce.d*)
    }
    .bss (NOLOAD) : AT(ADDR(.bss) - 0xFFFF000000000000) {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    _end = .;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
__bss_size = (__bss_end - __bss_start)>>3;
