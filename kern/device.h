#ifndef K_DEVICE_H
#define K_DEVICE_H

#include <kern/lib.h>

struct dev_header {
    /* Store some information about where the device was defined. */
    const char *name;
    const char *file;
    uint64_t    line;

    /* Store a singly-linked list of installed device drivers so we can
     * print them all out. */
    struct dev_header *next;
};

#define DECLARE_DEVICE(name, ...) \
struct name ## _device { \
    struct dev_header device_header; \
    __VA_ARGS__ \
}; \
extern struct name ## _device *name ## _dev;

#define IMPL_DEVICE(dev_name, own_name, ...) \
struct dev_name ## _device own_name = { \
    .device_header.name = #own_name, \
    .device_header.file = __FILE__, \
    .device_header.line = __LINE__, \
    __VA_ARGS__ \
};

#define DEFINE_DEVICE(dev_name, ...) \
struct dev_name ## _device dev_name ## _nulldev = { \
    .device_header.name = #dev_name "_nulldev", \
    .device_header.file = __FILE__, \
    .device_header.line = __LINE__, \
    __VA_ARGS__ \
}; \
struct dev_name ## _device *dev_name ## _dev = &dev_name ## _nulldev;

extern struct dev_header *device_list;

#define INSTALL_DEVICE(name, own_name) do { \
    name ## _dev = &own_name; \
    name ## _dev->device_header.next = device_list; \
    device_list = &name ## _dev->device_header; \
} while(0)

extern void load_devices();
extern void device_print_all();

#endif