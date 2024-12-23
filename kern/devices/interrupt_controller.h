#ifndef K_DEVICES_INTC_H
#define K_DEVICES_INTC_H

struct intc_device {
    void (*handle)(void);
};

extern struct intc_device *intc_dev;

#endif