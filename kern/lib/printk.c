#include <kern/lib.h>

#include <printf.h>
#include <kern/devices/console.h>

static void
printk_writefn(const char *data, size_t len, void *userdata) {
    (void)userdata;
    (void)len;

    while(*data) {
        console_dev->con_putc(*data);
        ++data;
    }
}

void
vprintk(const char *fmt, va_list va) {
    generic_printf(&printk_writefn, NULL, fmt, va);
}

void 
printk(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);

    vprintk(fmt, va);    

    va_end(va);
}