#ifndef K_DEVICES_POWER_H
#define K_DEVICES_POWER_H

struct power_attach {
    void (*shutdown)(void);
    void (*reboot)(void);
};

extern struct power_attach *the_power;

#endif