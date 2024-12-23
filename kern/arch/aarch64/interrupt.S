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
    sub sp, sp, #288
    str x0, [sp, #8]
    mov x0, #\id
    str x0, [sp, #0]
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

    stp x1,  x2,  [sp, #16 * 1]
    stp x3,  x4,  [sp, #16 * 2]
    stp x5,  x6,  [sp, #16 * 3]
    stp x7,  x8,  [sp, #16 * 4]
    stp x9,  x10, [sp, #16 * 5]
    stp x11, x12, [sp, #16 * 6]
    stp x13, x14, [sp, #16 * 7]
    stp x15, x16, [sp, #16 * 8]
    stp x17, x18, [sp, #16 * 9]
    stp x19, x20, [sp, #16 * 10]
    stp x21, x22, [sp, #16 * 11]
    stp x23, x24, [sp, #16 * 12]
    stp x25, x26, [sp, #16 * 13]
    stp x27, x28, [sp, #16 * 14]
    stp x29, x30, [sp, #16 * 15]
    
    /* Get the esr, elr, spsr, and far registers.
     * TODO: These will probably be different for the el0 interrupts. */
    mrs x0, esr_el1
    mrs x1, elr_el1
    mrs x2, spsr_el1
    mrs x3, far_el1

    /* Store the esr, elr, spsr, far registers. */
    stp x0, x1, [sp, #16 * 16]
    stp x2, x3, [sp, #16 * 17]

    /* The trapframe struct is pointed to by SP, put it in the first arg */
    mov x0, sp
    bl aarch64_trapentry

    /* We must restore the value of elr and spsr before we eret. */
    ldp x0, x1, [sp, #16 * 16]
    ldp x2, x3, [sp, #16 * 17]

    // No need to restore esr or far. TODO: Maybe store these in a more efficient
    // order in the trapframe.
    // msr esr_el1, x0
    msr elr_el1, x1
    msr spsr_el1, x2
    // msr far_el1, x3

    /* restore all the registers */

    /* TODO: Store floating point registers. */

    ldp x1,  x2,  [sp, #16 * 1]
    ldp x3,  x4,  [sp, #16 * 2]
    ldp x5,  x6,  [sp, #16 * 3]
    ldp x7,  x8,  [sp, #16 * 4]
    ldp x9,  x10, [sp, #16 * 5]
    ldp x11, x12, [sp, #16 * 6]
    ldp x13, x14, [sp, #16 * 7]
    ldp x15, x16, [sp, #16 * 8]
    ldp x17, x18, [sp, #16 * 9]
    ldp x19, x20, [sp, #16 * 10]
    ldp x21, x22, [sp, #16 * 11]
    ldp x23, x24, [sp, #16 * 12]
    ldp x25, x26, [sp, #16 * 13]
    ldp x27, x28, [sp, #16 * 14]
    ldp x29, x30, [sp, #16 * 15]
    ldr x0, [sp, #8]

    /* Free stack space allocated in 'define_entry' */
    add sp, sp, #288

    eret
