#include "bcm2711_irq.h"

#define BASE_ADDRESS (0x40000000 + (0xFE000000 - 0xC0000000))

#define ARM_LOCAL (BASE_ADDRESS +   0x01800000)

#define TIMER_CNTRL0 (*(volatile uint32_t*)(ARM_LOCAL + 0x40))

void
bcm2711_irq_init() {
    
}

void
bcm2711_irq_enable_timer() {
    TIMER_CNTRL0 |= 1;
}