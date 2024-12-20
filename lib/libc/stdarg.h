#ifndef LIBC_STDARG_H
#define LIBC_STDARG_H

/* Here we essentially have to defer to a compiler builtin. */

#ifndef __GNUC__
    #error "stdarg.h only supported for gnu C"
#endif

typedef __builtin_va_list va_list;

#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8)
#define va_start(ap, fmt)  __builtin_stdarg_start(ap, fmt)
#else
#define va_start(ap, fmt)  __builtin_va_start(ap, fmt)
#endif
#define va_arg(ap,t) __builtin_va_arg(ap,t)
#define va_copy(ap1,ap2) __builtin_va_copy(ap1,ap2)
#define va_end(ap) __builtin_va_end(ap)

#endif