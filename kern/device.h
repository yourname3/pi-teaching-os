#ifndef K_DEVICE_H
#define K_DEVICE_H

#include <kern/lib.h>

#define DECLARE_DEVICE(name, ...) \
struct name ## _device { \
    const char *dev_name; \
    const char *dev_file; \
    uint64_t    dev_line; \
    __VA_ARGS__ \
}; \
extern struct name ## _device *name ## _dev;

#define IMPL_DEVICE(name, own_name, ...) \
struct name ## _device own_name = { \
    .dev_name = #own_name, \
    .dev_file = __FILE__, \
    .dev_line = __LINE__, \
    __VA_ARGS__ \
};

#define DEFINE_DEVICE(name, ...) \
struct name ## _device name ## _nulldev = { \
    .dev_name = #name "_nulldev", \
    .dev_file = __FILE__, \
    .dev_line = __LINE__, \
    __VA_ARGS__ \
}; \
struct name ## _device *name ## _dev = &name ## _nulldev;

extern void load_devices();
extern void device_print_all();

#endif