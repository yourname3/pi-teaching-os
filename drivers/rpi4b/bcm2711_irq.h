#ifndef RPI4B_BCM2711_IRQ_H
#define RPI4B_BCM2711_IRQ_H

#include <kern/types.h>

void bcm2711_irq_init();
void bcm2711_irq_enable_timer();

#endif