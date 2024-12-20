#include "trapframe.h"

#include <kern/lib.h>

void aarch64_trapentry(struct trapframe *tf) {
    /* Last two bits indicate interrupt type */
    switch(tf->vector_id & 3) {
        case 0: printk("Synchronous"); break;
        case 1: printk("IRQ"); break;
        case 2: printk("FIQ"); break;
        case 3: printk("SError"); break;
    }
    printk(": ");

    /* Similar setup to https://github.com/bztsrc/raspi3-tutorial/blob/master/11_exceptions/exc.c for now */

    uint64_t type = (tf->esr >> 26);
    switch(type) {
        case 0b000000: printk("Unknown"); break;
        case 0b000001: printk("Trapped WFI/WFE"); break;
        case 0b001110: printk("Illegal execution"); break;
        case 0b010101: printk("System call"); break;
        case 0b100000: printk("Instruction abort, lower EL"); break;
        case 0b100001: printk("Instruction abort, same EL"); break;
        case 0b100010: printk("Instruction alignment fault"); break;
        case 0b100100: printk("Data abort, lower EL"); break;
        case 0b100101: printk("Data abort, same EL"); break;
        case 0b100110: printk("Stack alignment fault"); break;
        case 0b101100: printk("Floating point"); break;
        default: printk("Unknown"); break;
    }

    if(type == 0b100100 || type == 0b100101) {
        printk(", ");
        switch((tf->esr >> 2) & 3) {
            case 0: printk("Address size fault"); break;
            case 1: printk("Translation fault"); break;
            case 2: printk("Access flag fault"); break;
            case 3: printk("Permission fault"); break;
        }
        printk(" at level %d", (tf->esr & 3));
    }
    else {
        printk(" ");
    }

    printk("ESR_EL1 = %p ELR_EL1 = %p SPSR_EL1 = %p FAR_EL1 = %p",
        tf->esr, tf->elr, tf->spsr, tf->far);
}