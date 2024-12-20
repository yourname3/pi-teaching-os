.section ".data.boot"

.balign 0x1000
_init_pagetable_level1:
    .quad 1 | (1 << 10) | (3 << 8)
    /* Place a second entry for the RPI peripheral MMIO. This should have nRnEnG properties. */
    .quad 1 | (1 << 10) | (3 << 8) | (1 << 2) | 0xC0000000
    .zero 8 * (512 - 2)

.balign 0x1000
_init_pagetable_level0:
    /* .quad 3 | (1 << 10) | (3 << 8) */
    .quad 3
    .zero 8 * (512 - 1)

.section .data
.balign 0x1000
_init_kstack_bottom:
    .zero 8192
_init_kstack_top: /* Stack grows downwards. */

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

    msr     spsel, 1
    mrs     x0, CurrentEL
    cmp     x0, 0b0100
    beq     in_el1
    blo     in_el0
    cmp     x0, 0b1000
    beq     in_el2
    b       in_el3
in_el0:
    b       .
in_el3:
    mrs     x0, scr_el3
    orr     x0, x0, (1 << 10)
    orr     x0, x0, (1 << 0)
    and     x0, x0, ~(1 << 3)
    and     x0, x0, ~(1 << 2)
    and     x0, x0, ~(1 << 1)
    msr     scr_el3, x0
    mov     x0, 0b01001
    msr     spsr_el3, x0

    adr     x0, in_el2
    msr     elr_el3, x0
    eret
in_el2:
    mrs     x0, hcr_el2
    orr     x0, x0, (1 << 31)
    and     x0, x0, ~(1 << 5)
    and     x0, x0, ~(1 << 4)
    and     x0, x0, ~(1 << 3)
    msr     hcr_el2, x0
    mov     x0, 0b00101
    msr     spsr_el2, x0

    adr     x0, in_el1
    msr     elr_el2, x0
    eret
in_el1:
    mov     x0, 0b0101
    msr     spsr_el1, x0


    ldr x0, =0xFF
    msr mair_el1, x0
    isb

    ldr x0, =_init_pagetable_level1
    ldr x1, _init_pagetable_level0
    orr x1, x1, x0

    ldr x0, =_init_pagetable_level0
    str x1, [x0]
   
    ldr x0, =_init_pagetable_level0
    ldr x1, =((0b101 << 32) | (2 << 30) | (3 << 28) | (1 << 26) | (1 << 24) | (16 << 16) | (0 << 14) | (3 << 12) | (1 << 10) | (1 << 8) | (16))

    msr ttbr0_el1, x0 // Set TTBR0
    msr ttbr1_el1, x0 // Set TTBR1
    msr tcr_el1, x1 // Set TCR
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
    ldr     x1, =_init_kstack_top
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
