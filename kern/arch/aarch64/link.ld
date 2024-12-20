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
    . = 0x80000;     /* Kernel load address for AArch64 */
    /* .text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*) } */
    .text.boot : { KEEP(*(.text.boot)) }
    . = 0xFFFF000000000000;
    .text : { *(.text .text.* .gnu.linkonce.t*) }
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    PROVIDE(_data = .);
    .data : { *(.data .data.* .gnu.linkonce.d*) }
    .bss (NOLOAD) : {
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
