MEMORY {
    phys (rwxla) : ORIGIN = 0x80000, LENGTH = 1024M
    virt (rwxa) : ORIGIN = 0xFFFF000000000000, LENGTH = 1024M
}

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

    .text.boot : { KEEP(*(.text.boot)) } > phys AT> phys

    .text : { *(.text .text.* .gnu.linkonce.t*) } > virt AT> phys
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) } > virt AT> phys
    PROVIDE(_data = .);
    .data :  {
        *(.data .data.* .gnu.linkonce.d*)
    } > virt AT> phys
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    } > virt AT> phys
    _end = .;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
__bss_size = (__bss_end - __bss_start)>>3;
