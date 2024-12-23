#include "gic_400.h"

#include <kern/lib.h>
#include <kern/irq.h>
#include <kern/devices/interrupt_controller.h>
#include <kern/devices/preempt.h>

static volatile uint32_t *gicd_dist_base = 0;
static volatile uint32_t *gicc_cpu_base = 0;

static volatile uint32_t *gicd_enable_irq_base = 0;

static volatile uint32_t *gicc_iar = 0;
static volatile uint32_t *gicc_eoir = 0;

static volatile uint32_t *gic_irq_target_base = 0;

static size_t *irq_to_kirq_map = NULL;
static size_t irq_map_count = 0;

/*
 * ref: https://github.com/s-matyukevich/raspberry-pi-os/issues/237
 */

void
gic_400_enable(uint32_t irq) {
    uint32_t n = irq / 32;
    uint32_t offset = irq % 32;
    volatile uint32_t *enable_reg = &gicd_enable_irq_base[n];
    /* Register is set-enable so we don't OR it. */
    *enable_reg = (1 << offset);

    //printk("gic_400_enable %d - %x\n", n, *enable_reg);
}

void
gic_400_assign_irq_cpu(uint32_t irq, uint32_t cpu) {
    uint32_t n = irq / 4;
    volatile uint32_t *target_reg = &gic_irq_target_base[n];

    uint32_t offset = (irq % 4) * 8 + cpu;

    /* TODO support cpu param */
    *target_reg |= (1 << offset);

    //printk("gic_400_assign %d - %x\n", n, *target_reg);
}

void
gic_400_handle(void) {
    /* Get IRQ number from the IAR register. */
    uint32_t irq = *gicc_iar & 0x2ff;

    bool preempt = false;

    /* Handle IRQs in a loop until we see the "spurious" value of 1023,
     * which indicates that there are no more pending interrupts. */
    do {
        /* Fire any IRQ that we know of. */
        if(irq < irq_map_count) {
            int iact;
            /* We don't really care about the result right now. */
            if(irq_fire(irq_to_kirq_map[irq], &iact)) {
                if(iact == IACT_PREEMPT) { preempt = true; }
            }
        }

        /* Signal the end of interrupt to the GIC. */
        *gicc_eoir = irq;

        /* Read the next interrupt from the IAR. */
        irq = *gicc_iar;
    } while(irq != 1023);

    if(preempt) {
        preempt_fire();
    }
}

IMPL_DEVICE(intc, gic_400,
    .handle = gic_400_handle
);

void
gic_400_install_ppi(uint32_t ppi, size_t kern_irq) {
    /* For PPIs, we do not need to assign the IRQ to a CPU, but we do need
     * to enable it. */
    gic_400_enable(ppi);

    if(kern_irq < irq_map_count) {
        /* Put the IRQ in the map. */
        irq_to_kirq_map[ppi] = kern_irq;
    }
}

void
gic_400_init(uintptr_t base_addr, size_t total_gic_irqs) {
    gicd_dist_base = (volatile uint32_t*)(base_addr + 0x1000);
    gicc_cpu_base = (volatile uint32_t*)(base_addr + 0x2000);

    gicd_enable_irq_base = (gicd_dist_base + 0x100 / 4);
    
    gicc_iar = gicc_cpu_base + 0xC/4;
    gicc_eoir = gicc_cpu_base + 0x10/4;

    gic_irq_target_base = gicd_dist_base + 0x800/4;

    volatile uint32_t *gicd_ctlr = gicd_dist_base + 0;
    volatile uint32_t *gicc_ctlr = gicc_cpu_base + 0;
    volatile uint32_t *gicc_prio = gicc_cpu_base + 4;

    intc_dev = &gic_400;

    /* Allocate an array allowing us to map our internal IRQ numbers to the
     * kernel IRQ numbers. By default, these are all IRQ_NULL, which is
     * perfect--all our internal IRQs fire no kernel IRQs. */
    irq_to_kirq_map = kzalloc_or_die(total_gic_irqs,
        "failed to allocate gic_400 irq_to_kirq map");
    irq_map_count = total_gic_irqs;

    /* Enable */
    //*gicd_ctlr = 7;
    //*gicc_ctlr = 3;

    /* All interrupts */
    //*gicc_prio = 0xFF;
}