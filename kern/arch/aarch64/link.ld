SECTIONS
{
    /* Start putting stuff at the address expected by the bootloader.
     * For Raspberry Pi, this is 0x80000. */
    . = 0x80000;

    /* We're putting everything in physical memory starting at the boot address.
     * However, we then have a split between physical memory and virtual memory:
     * Everything but the bootloader will be running with some memory mapping
     * occuring.
     *
     * But the boot code does not have the MMU set up, so it needs to run at
     * the low addresses.
     *
     * Put the boot code in ".text" and all other code in ".text.kern" -- this
     * makes QEMU happy when booting the kernel. */
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
    . = ALIGN(4096);
    
    kern_text_start = .;
    .text.kern : AT(ADDR(.text.kern) - 0xFFFF000000000000) {
        
        *(.text .text.* .gnu.linkonce.t*)
        
    }
    kern_text_end = .;

    . = ALIGN(4096);
    kern_rodata_start = .;
    .rodata : AT(ADDR(.rodata) - 0xFFFF000000000000) { *(.rodata .rodata.* .gnu.linkonce.r*) }
    kern_rodata_end = .;

    . = ALIGN(4096);
    kern_data_start = .;
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
    kern_data_end = .;
    _end = .;

   /DISCARD/ : {
      *(.comment)
      *(.note*)
      *(.eh_frame*)
      *(.discard)
      *(.discard.*)
      *(.interp .dynamic)
      *(.dynsym .dynstr .hash)
   }
}
__bss_size = (__bss_end - __bss_start)>>3;
