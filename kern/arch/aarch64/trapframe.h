#ifndef AARCH64_TRAPFRAME_H
#define AARCH64_TRAPFRAME_H

#include <kern/types.h>

struct trapframe {
    uint64_t vector_id;

    uint64_t x[32];

    uint64_t esr;
    uint64_t elr;
    uint64_t spsr;
    uint64_t far;
};

#endif