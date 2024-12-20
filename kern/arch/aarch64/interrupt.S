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

    /* The trapframe struct is pointed to by SP, put it in the first arg */
    mov x0, sp
    bl aarch64_trapentry

    /* restore all the registers */
    eret
