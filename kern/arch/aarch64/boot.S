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
    /* Setup the GIC (TODO replace with C code) */
    bl setup_gic

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

    /* Disable TGE and E2H Bits.
     * TODO: Figure out exactly which bits we want set.
     * https://krinkinmu.github.io/2021/01/10/aarch64-interrupt-handling.html
     */
    and x0, x0, #~(1 << 34)
    and x0, x0, #~(1 << 27)

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
3:  cbz     w2, ready                // Quit loop if zero
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b                // Loop if non-zero

ready:
    b high_code

#define GICC_CTRLR	0x0
#define GICC_PMR	0x4
#define IT_NR		0x8	// Number of interrupt enable registers (256 total irqs)
#define GICD_CTRLR	0x0
#define GICD_IGROUPR	0x80
#define GIC_DISTB		0xff841000
#define GIC_CPUB		0xff842000

/* TODO:
 * This code is taken from the MIT licensed RPI "armstub8.S" file. It should be possible to translate
 * this to C and put it into the GIC driver. */
setup_gic:				// Called from secure mode - set all interrupts to group 1 and enable.
	mrs	x0, MPIDR_EL1
	ldr	x2, =GIC_DISTB
	tst	x0, #0x3
	b.eq	2f			// primary core

	mov	w0, #3			// Enable group 0 and 1 IRQs from distributor
	str	w0, [x2, #GICD_CTRLR]
2:
	add	x1, x2, #(GIC_CPUB - GIC_DISTB)
	mov	w0, #0x1e7
	str	w0, [x1, #GICC_CTRLR]	// Enable group 1 IRQs from CPU interface
	mov	w0, #0xff
	str	w0, [x1, #GICC_PMR]	// priority mask
	add	x2, x2, #GICD_IGROUPR
	mov	x0, #(IT_NR * 4)
	mov	w1, #~0			// group 1 all the things
3:
	subs	x0, x0, #4
	str	w1, [x2, x0]
	b.ne	3b
	ret

.section ".text"

    // Start executing in the higher half now
high_code:

    // Set up interrupts
    ldr x0, =interrupt_vectors
    msr vbar_el1, x0

    // Enable all exceptions
    msr daifclr, #0xF

    // Jump to our main() routine in C (make sure it doesn't return)
4:  bl      main
    // In case it does return, halt the main core too
    b       1b
