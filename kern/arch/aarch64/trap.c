#include "trapframe.h"

#include <kern/lib.h>

void aarch64_trapentry(struct trapframe *tf) {
    printk("in trapentry (tf = %p)", tf);
}