#include "gic_400.h"

#include <kern/lib.h>

static volatile uint32_t *gicd_dist_base = 0;
static volatile uint32_t *gicc_cpu_base = 0;

static volatile uint32_t *gicd_enable_irq_base = 0;

static volatile uint32_t *gicc_iar = 0;
static volatile uint32_t *gicc_eoir = 0;

static volatile uint32_t *gic_irq_target_base = 0;

#define GIC_IRQS        192
#define GIC_SPURIOUS    1023
#define GICD_BASE       ((0xFF841000 - 0xC0000000) + 0x40000000)
#define GICD_CTLR       (GICD_BASE + 0x000)
#define GICD_ENABLE     (GICD_BASE + 0x100)
#define GICD_DISABLE    (GICD_BASE + 0x180)
#define GICD_PEND_CLR   (GICD_BASE + 0x280)
#define GICD_ACTIVE_CLR (GICD_BASE + 0x380)
#define GICD_PRIO       (GICD_BASE + 0x400)
#define GICD_TARGET     (GICD_BASE + 0x800)
#define GICD_CFG        (GICD_BASE + 0xc00)
#define GICD_SGIR       (GICD_BASE + 0xF00)
// GIC-400 CPU Interface Controller
#define GICC_BASE       ((0xFF841000 - 0xC0000000) + 0x40000000)
#define GICC_CTLR       (GICC_BASE + 0x000)
#define GICC_PRIO       (GICC_BASE + 0x004)
#define GICC_ACK        (GICC_BASE + 0x00c)
#define GICC_EOI        (GICC_BASE + 0x010)

void PUT32(unsigned long addr, unsigned int value) {
    volatile unsigned int *ptr = (volatile unsigned int*)addr;
    *ptr = value;
}

unsigned int GET32(unsigned long addr) {
    volatile unsigned int *ptr = (volatile unsigned int*)addr;
    return *ptr;
}

// initialize GIC-400
void gic_init (void) {
    unsigned int i;
    
    // disable Distributor and CPU interface
    PUT32(GICD_CTLR, 0);
    PUT32(GICC_CTLR, 0);

    // disable, acknowledge and deactivate all interrupts
    for (i = 0; i < (GIC_IRQS/32); ++i) {
        PUT32(GICD_DISABLE     + 4*i, ~0);
        PUT32(GICD_PEND_CLR    + 4*i, ~0);
        PUT32(GICD_ACTIVE_CLR  + 4*i, ~0);
    }

    // direct all interrupts to core 0 (=01) with default priority a0
    for (i = 0; i < (GIC_IRQS/4); ++i) {
        PUT32(GICD_TARGET + 4*i, 0x01010101);
        PUT32(GICD_PRIO   + 4*i, 0xa0a0a0a0);
    }

    // config all interrupts to level triggered
    for (i = 0; i < (GIC_IRQS/16); ++i) {
        PUT32(GICD_CFG + 4*i, 0);
    }

    // enable Distributor
    PUT32(GICD_CTLR, 7);

    // set Core0 interrupts mask theshold prio to F0, to react on higher prio a0
    PUT32(GICC_PRIO, 0xFF);
    // enable CPU interface
    PUT32(GICC_CTLR, 0x1e7);
}

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

    gic_init();
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