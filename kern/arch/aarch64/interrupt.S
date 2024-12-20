.section ".text"

/**
 * Each interrupt handler should to the following:
 * 1. Allocate (32 + 5) * 8 bytes on the stack
 * 2. Move its specific identifier into the top slot (which requires also
      saving the x0 register first)
 * 3. Jump to finish_trap
 */
.macro define_entry id
.align 7
    sub sp, sp, #296
    str x0, [sp, #280]
    mov x0, #\id
    str x0, [sp, #288]
    b finish_trap
.endm

    .global interrupt_vectors
    .align 11
interrupt_vectors:
    define_entry 0
    define_entry 1
    define_entry 2
    define_entry 3

    define_entry 4
    define_entry 5
    define_entry 6
    define_entry 7

    define_entry 8
    define_entry 9
    define_entry 10
    define_entry 11

    define_entry 12
    define_entry 13
    define_entry 14
    define_entry 15


finish_trap:
    /* store all the registers */

    str x1,  [sp, #0x110]
    str x2,  [sp, #0x108]
    str x3,  [sp, #0x100]
    str x4,  [sp, #0xF8]
    str x5,  [sp, #0xF0]
    str x6,  [sp, #0xE8]
    str x7,  [sp, #0xE0]
    str x8,  [sp, #0xD8]
    str x9,  [sp, #0xD0]
    str x10, [sp, #0xC8]
    str x11, [sp, #0xC0]
    str x12, [sp, #0xB8]
    str x13, [sp, #0xB0]
    str x14, [sp, #0xA8]
    str x15, [sp, #0xA0]
    str x16, [sp, #0x98]
    str x17, [sp, #0x90]
    str x18, [sp, #0x88]
    str x19, [sp, #0x80]
    str x20, [sp, #0x78]
    str x21, [sp, #0x70]
    str x22, [sp, #0x68]
    str x23, [sp, #0x60]
    str x24, [sp, #0x58]
    str x25, [sp, #0x50]
    str x26, [sp, #0x48]
    str x27, [sp, #0x40]
    str x28, [sp, #0x38]
    str x29, [sp, #0x30]
    str x30, [sp, #0x28]
    
    /* Get the esr, elr, spsr, and far registers.
     * TODO: These will probably be different for the el0 interrupts. */
    mrs x0, esr_el1
    mrs x1, elr_el1
    mrs x2, spsr_el1
    mrs x3, far_el1

    /* Store the esr, elr, spsr, far registers. */
    str x27, [sp, #0x20]
    str x28, [sp, #0x18]
    str x29, [sp, #0x10]
    str x30, [sp, #0x08]

    /* The trapframe struct is pointed to by SP, put it in the first arg */
    mov x0, sp
    /* Oops, it's actually off by 8 right now. */
    add x0, x0, #8
    bl aarch64_trapentry

    /* restore all the registers */

    /* TODO: Store floating point registers. */

    ldr x1,  [sp, #0x110]
    ldr x2,  [sp, #0x108]
    ldr x3,  [sp, #0x100]
    ldr x4,  [sp, #0xF8]
    ldr x5,  [sp, #0xF0]
    ldr x6,  [sp, #0xE8]
    ldr x7,  [sp, #0xE0]
    ldr x8,  [sp, #0xD8]
    ldr x9,  [sp, #0xD0]
    ldr x10, [sp, #0xC8]
    ldr x11, [sp, #0xC0]
    ldr x12, [sp, #0xB8]
    ldr x13, [sp, #0xB0]
    ldr x14, [sp, #0xA8]
    ldr x15, [sp, #0xA0]
    ldr x16, [sp, #0x98]
    ldr x17, [sp, #0x90]
    ldr x18, [sp, #0x88]
    ldr x19, [sp, #0x80]
    ldr x20, [sp, #0x78]
    ldr x21, [sp, #0x70]
    ldr x22, [sp, #0x68]
    ldr x23, [sp, #0x60]
    ldr x24, [sp, #0x58]
    ldr x25, [sp, #0x50]
    ldr x26, [sp, #0x48]
    ldr x27, [sp, #0x40]
    ldr x28, [sp, #0x38]
    ldr x29, [sp, #0x30]
    ldr x30, [sp, #0x28]

    /* Free stack space allocated in 'define_entry' */
    add sp, sp, #296

    eret
