#ifndef K_DEVICES_POWER_H
#define K_DEVICES_POWER_H

#include <kern/device.h>

DECLARE_DEVICE(power,
    void (*shutdown)(void);
    void (*reboot)(void);
    void (*panic)(void);
);

#endif