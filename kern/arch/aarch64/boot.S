.section ".data.boot"

/* _init_pagetable_level1:
    .quad 1 | (1 << 10) | (3 << 8)
    .zero 8 * (512 - 1)

_init_pagetable_level0:
    .quad 3 | (1 << 10) | (3 << 8)
    .zero 8 * (512 - 1) */

_init_pagetable_level1:
    .zero 8 * 512
_init_pagetable_level0:
    .zero 8 * 512

.section .data
_init_kstack:
    .zero 8192

// We place the .text.boot section first in our linker script to ensure that the
// boot code is executed first.
.section ".text.boot"

.global _start

_start:
    // Check processor ID is zero (executing on main core), else hang
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // We're not on the main core, so hang in an infinite wait loop
1:  wfe
    b       1b
2:  // We're on the main core!

    // Before we can jump to the kernel main code, we have to set up a basic
    // memory map.
    //
    // The kernel is mapped to the "higher half" starting at 0xFFFF0000_00000000.
    // On ARM, this has some convenient properties, in particular that we get
    // essentially a separate memory map for both the high address and low
    // address.

    ldr x0, =0xFF
    msr mair_el1, x0

    ldr x0, =_init_pagetable_level1
    ldr x1, _init_pagetable_level0
    orr x1, x1, x1

    ldr x0, =_init_pagetable_level0
    str x1, [x0]
   
    ldr x2, =((0b101 << 32) | (2 << 30) | (3 << 28) | (1 << 26) | (1 << 24) | (16 << 16) | (0 << 14) | (3 << 12) | (1 << 10) | (1 << 8) | (16))

    msr ttbr0_el1, x0 // Set TTBR0
    msr ttbr1_el1, x0 // Set TTBR1
    msr tcr_el1, x2 // Set TCR
    isb // The ISB forces these changes to be
    // seen before the MMU is enabled.
    mrs x0, sctlr_el1 // Read System Control Register
    // configuration data
    orr x0, x0, #1 // Set [M] bit and enable the MMU.
    msr sctlr_el1, x0 // Write System Control Register
    // configuration data
    isb // The ISB forces these changes to be
    // seen by the next instruction

    // Set stack to start below our code
    ldr     x1, =_init_kstack
    mov     sp, x1

    // Clean the BSS section
    ldr     x1, = __bss_start     // Start address
    ldr     w2, = __bss_size      // Size of the section
3:  cbz     w2, 4f                // Quit loop if zero
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b                // Loop if non-zero

    // Jump to our main() routine in C (make sure it doesn't return)
4:  bl      main
    // In case it does return, halt the main core too
    b       1b
