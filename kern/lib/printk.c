#include <kern/lib.h>

#include <kern/console/console.h>
#include <printf.h>

static void
printk_writefn(const char *data, size_t len, void *userdata) {
    (void)userdata;
    (void)len;

    console_putstr(data);
}

void 
printk(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);

    generic_printf(&printk_writefn, NULL, fmt, va);

    va_end(va);
}