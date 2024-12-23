#ifndef K_DEVICES_INTC_H
#define K_DEVICES_INTC_H

#include <kern/device.h>

DECLARE_DEVICE(intc,
    void (*handle)(void);
);

#endif