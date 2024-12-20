#ifndef LIBC_PRINTF_H
#define LIBC_PRINTF_H

#include <stddef.h>
#include <stdarg.h>

/* printf.h -- not strictly speaking a part of libc, but instead provides an
 * interface to our generic_printf function. */

typedef void (*printf_write_fn)(const char *data, size_t bytes, void *userdata);

size_t generic_printf(printf_write_fn write, void *userdata, const char *fmt, va_list args); 

#endif