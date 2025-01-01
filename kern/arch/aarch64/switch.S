
.section ".text"

.global aarch64_switch

aarch64_switch:
    // Some notes on AARCH64:
    // There are 32 registers x0-x31.
    // x31 is the stack pointer.
    // x30 is the "procedure link register" i.e. the return address.
    // x29 is (conventionally) the frame pointer.
    //
    // The rest of the x registers are, for the purposes of a context switch,
    // machine-independent.
    //
    // What we have to do to perform a switch is:
    // 1. Save all the registers onto our current stack.
    // 2. Load the target task's stack pointer into sp.
    // 3. Reload all registers from the (new) stack.
    // 4. Return using the (loaded) return address. This doesn't actually
    //    require anything besides the normal 'ret'.

    // As per the calling convention, the two parameters (task1->sp and task2->sp)
    // are stored in x0 and x1.

    sub sp, sp, #256 // Allocate enough space for 32 8-bit registers (aligned to 16 bytes)

    

    // Store all the 64 bit registers onto the stack.
    str x30, [sp, #0xF0]
    str x29, [sp, #0xE8]
    str x28, [sp, #0xE0]
    str x27, [sp, #0xD8]
    str x26, [sp, #0xD0]
    str x25, [sp, #0xC8]
    str x24, [sp, #0xC0]
    str x23, [sp, #0xB8]
    str x22, [sp, #0xB0]
    str x21, [sp, #0xA8]
    str x20, [sp, #0xA0]
    str x19, [sp, #0x98]
    str x18, [sp, #0x90]
    str x17, [sp, #0x88]
    str x16, [sp, #0x80]
    str x15, [sp, #0x78]
    str x14, [sp, #0x70]
    str x13, [sp, #0x68]
    str x12, [sp, #0x60]
    str x11, [sp, #0x58]
    str x10, [sp, #0x50]
    str x9,  [sp, #0x48]
    str x8,  [sp, #0x40]
    str x7,  [sp, #0x38]
    str x6,  [sp, #0x30]
    str x5,  [sp, #0x28]
    str x4,  [sp, #0x20]
    str x3,  [sp, #0x18]
    str x2,  [sp, #0x10]
    str x1,  [sp, #0x08]
    str x0,  [sp, #0x00]

    

    // Use x2 to copy to and from sp.
    mov x2, sp

    // Store the final value of sp into the previous task struct (pointed to by x0).
    str x2, [x0, #0]

    

    // Load sp from the new task struct (pointed to by x1).
    ldr x2, [x1, #0]

    // ON REAL HARDWARE: We make it here...

    mov sp, x2

    // but we don't make it here... (or at least we can't call aarch64_halfway from here.)

    // Reload all the state registers 
    ldr x30, [sp, #0xF0]
    ldr x29, [sp, #0xE8]
    ldr x28, [sp, #0xE0]
    ldr x27, [sp, #0xD8]
    ldr x26, [sp, #0xD0]
    ldr x25, [sp, #0xC8]
    ldr x24, [sp, #0xC0]
    ldr x23, [sp, #0xB8]
    ldr x22, [sp, #0xB0]
    ldr x21, [sp, #0xA8]
    ldr x20, [sp, #0xA0]
    ldr x19, [sp, #0x98]
    ldr x18, [sp, #0x90]
    ldr x17, [sp, #0x88]
    ldr x16, [sp, #0x80]
    ldr x15, [sp, #0x78]
    ldr x14, [sp, #0x70]
    ldr x13, [sp, #0x68]
    ldr x12, [sp, #0x60]
    ldr x11, [sp, #0x58]
    ldr x10, [sp, #0x50]
    ldr x9,  [sp, #0x48]
    ldr x8,  [sp, #0x40]
    ldr x7,  [sp, #0x38]
    ldr x6,  [sp, #0x30]
    ldr x5,  [sp, #0x28]
    ldr x4,  [sp, #0x20]
    ldr x3,  [sp, #0x18]
    ldr x2,  [sp, #0x10]
    ldr x1,  [sp, #0x08]
    ldr x0,  [sp, #0x00]

    add sp, sp, #256 // Free stack space
    ret
