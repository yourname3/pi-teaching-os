#include "bcm2711_irq.h"

#define BASE_ADDRESS (0x40000000 + (0xFE000000 - 0xC0000000))

#define ARM_LOCAL (BASE_ADDRESS +   0x01800000)

#define TIMER_CNTRL0 (*(volatile uint32_t*)(ARM_LOCAL + 0x40))

#define IRQ_ENABLE_0 ((volatile uint32_t*)(BASE_ADDRESS + 0x0000B200 + 20))

/**
 * TODO:
 * It appears that the "legacy" interrupt controller is not going to just work,
 * probably--I'm not entirely sure what QEMU does but that's what I'm thinking.
 * 
 * So we probably want to just use the GIC-400 instead.
 */

void
bcm2711_irq_init() {
    
}

void
bcm2711_irq_enable_timer() {
    TIMER_CNTRL0 |= 1;
    *IRQ_ENABLE_0 = (2 | 8);
}