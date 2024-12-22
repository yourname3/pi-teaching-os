#include "gic_400.h"

#include <kern/lib.h>

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

    volatile uint32_t *gicd_ctlr = gicd_dist_base + 0;
    volatile uint32_t *gicc_ctlr = gicc_cpu_base + 0;
    volatile uint32_t *gicc_prio = gicc_cpu_base + 4;

    /* Enable */
    //*gicd_ctlr = 7;
    //*gicc_ctlr = 3;

    /* All interrupts */
    //*gicc_prio = 0xFF;
}

void
gic_400_enable(uint32_t irq) {
    uint32_t n = irq / 32;
    uint32_t offset = irq % 32;
    volatile uint32_t *enable_reg = &gicd_enable_irq_base[n];
    /* Register is set-enable so we don't OR it. */
    *enable_reg = (1 << offset);

    printk("gic_400_enable %d - %x\n", n, *enable_reg);
}

void
gic_400_assign_irq_cpu(uint32_t irq, uint32_t cpu) {
    uint32_t n = irq / 4;
    volatile uint32_t *target_reg = &gic_irq_target_base[n];

    uint32_t offset = (irq % 4) * 8 + cpu;

    /* TODO support cpu param */
    *target_reg |= (1 << offset);

    printk("gic_400_assign %d - %x\n", n, *target_reg);
}