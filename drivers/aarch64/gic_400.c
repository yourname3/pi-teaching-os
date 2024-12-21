#include "gic_400.h"

static volatile uint32_t *gicd_dist_base = 0;
static volatile uint32_t *gicc_cpu_base = 0;

static volatile uint32_t *gicd_enable_irq_base = 0;

static volatile uint32_t *gicc_iar = 0;
static volatile uint32_t *gicc_eoir = 0;

static volatile uint32_t *gic_irq_target_base = 0;

/*
 * ref: https://github.com/s-matyukevich/raspberry-pi-os/issues/237
 */

void
gic_400_init(uintptr_t base_addr) {
    gicd_dist_base = (volatile uint32_t*)(base_addr + 0x1000);
    gicc_cpu_base = (volatile uint32_t*)(base_addr + 0x2000);

    gicd_enable_irq_base = (gicd_dist_base + 0x100 / 4);
    
    gicc_iar = gicc_cpu_base + 0xC/4;
    gicc_eoir = gicc_cpu_base + 0x10/4;

    gic_irq_target_base = gicd_dist_base + 0x800/4;
}

void
gic_400_enable(uint32_t irq) {
    uint32_t n = irq / 32;
    uint32_t offset = irq % 32;
    volatile uint32_t *enable_reg = &gicd_enable_irq_base[n];
    /* Register is set-enable so we don't OR it. */
    *enable_reg = (1 << offset);
}

void
gic_400_assign_irq_cpu(uint32_t irq, uint32_t cpu) {
    uint32_t n = irq / 4;
    volatile uint32_t *target_reg = &gic_irq_target_base[n];

    /* TODO support cpu param */
    *target_reg |= (1 << 8);
}