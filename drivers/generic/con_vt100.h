#ifndef D_CON_VT100_H
#define D_CON_VT100_H

#include <kern/types.h>
#include <kern/devices/console.h>

#include <kern/device.h>

/* Create a wrapper DEVICE for the console. When we initialize our wrapper,
 * we take over console_dev, and a different device can drive the con_vt100_dev.
 */

DECLARE_DEVICE(con_vt100,
    void (*putc)(char c);
    bool (*poll)(char *out);
)

void con_vt100_init();

#endif