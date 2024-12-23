#ifndef AARCH64_GIC_400_H
#define AARCH64_GIC_400_H

#include <kern/types.h>

void gic_400_init(uintptr_t base_addr, size_t total_gic_irqs);

//void gic_400_enable(uint32_t irq);

//void gic_400_assign_irq_cpu(uint32_t irq, uint32_t cpu);

void gic_400_install_ppi(uint32_t ppi, size_t kern_irq);

#endif