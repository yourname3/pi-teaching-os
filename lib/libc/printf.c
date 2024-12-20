#include "printf.h"

#include <string.h>

/**
 * Defines the structure used to implement generic_printf. Keeps track of the
 * state so that the implementation is somewhat easy to follow.
 */
struct printf_runner {
    printf_write_fn write;
    void *userdata;

    va_list va;

    int in_fmt;
    int enable_base_prefix;
    long long base;

    size_t written;
};

static void
printf_do_print(struct printf_runner *pr, const char *prefix1, const char *prefix2, const char *what) {
    size_t prefix1_len = strlen(prefix1);
    size_t prefix2_len = strlen(prefix2);
    size_t what_len = strlen(what);

    pr->write(prefix1, prefix1_len, pr->userdata);
    pr->write(prefix2, prefix2_len, pr->userdata);
    pr->write(what, what_len, pr->userdata);

    pr->written += prefix1_len + prefix2_len + what_len;

    /* Whenever we do a print, reset the in_fmt flag to false. */
    pr->in_fmt = 0;
}

static const char*
numerals = "0123456789abcdef";

#define INT_BUF_SIZE ((sizeof(unsigned long long) * 8) / 3)

static void
printf_do_uint(struct printf_runner *pr, unsigned long long value, const char *sign_prefix) {
    char buf[INT_BUF_SIZE] = { 0 };

    /* Initialize 'at' at the end of the string because we decrement it before
     * writing to it, so we'll get that one extra char for the terminator. */
    char *at = &buf[INT_BUF_SIZE - 1];

    do {
        /* Back up in the string to "allocate" a char. */
        --at;

        /* Read the current digit from the string. */
        *at = numerals[value % pr->base];

        /* Make the number smaller so we can read the higher digits. */
        value /= pr->base;
    } while(value > 0); /* Once value hits 0, we're done. */

    const char *base_prefix = "";
    if(pr->enable_base_prefix) {
        if(pr->base == 16) base_prefix = "0x";
    }

    /* Print based on the prefix and the "at" value. */
    printf_do_print(pr, sign_prefix, base_prefix, at);
}

static void
printf_do_int(struct printf_runner *pr, long long value) {
    const char *prefix = "";
    
    if(value < 0) {
        prefix = "-";
        /* TODO handle MIN_LONG_LONG? */

        value = -value;
    }

    printf_do_uint(pr, (unsigned long long)value, prefix);
}

static void
printf_begin_fmt(struct printf_runner *pr) {
    pr->in_fmt = 1;

    /* Setup default formatting options. */
    pr->enable_base_prefix = 0;
    pr->base = 10;
}

static void
printf_step(struct printf_runner *pr, char next) {
    if(!pr->in_fmt) {
        /* If we're not currently in a format string, there's only two possibilities:
         * we see a %, starting a format string, or we don't, in which case we
         * just print that character. */
        if(next == '%') {
            /* Switch into in_fmt mode */
            printf_begin_fmt(pr);
            return;
        }

        /* Print the character */
        char buf[2] = { next, '\0' };
        printf_do_print(pr, "", "", buf);
        return;
    }

    /* Now we know we're in_fmt. So, handle all the possible formatting options. */
    switch(next) {
        case 'd': pr->base = 10; printf_do_int(pr, va_arg(pr->va, long long)); return;
        case 'p':
            void *arg = va_arg(pr->va, void*);
            if(arg == NULL) {
                printf_do_print(pr, "", "", "(null)");
            }
            else {
                pr->enable_base_prefix = 1;
                pr->base = 16;
                printf_do_uint(pr, (unsigned long long)arg, "");
            }
            return;
            
        case 'x':
            pr->base = 16;
            printf_do_int(pr, va_arg(pr->va, long long));
            return;
        default:
            /* For anything that isn't otherwise handled, treat it as essentially
             * '%%' -- i.e. we just print it as a plain character. */
            {
                char buf[2] = { next, '\0' };
                printf_do_print(pr, "", "", buf);
            }
            return;
    }
}

size_t
generic_printf(printf_write_fn write, void *userdata, const char *fmt, va_list args) {
    struct printf_runner pr = {
        .write = write,
        .userdata = userdata,

        .in_fmt = 0,

        .written = 0,
    };

    va_copy(pr.va, args);

    /* Handle all characters in the format string. */
    for(;;) {
        /* Don't bother handling NUL terminators in step(). */
        if(!*fmt) break;
        printf_step(&pr, *fmt);
        ++fmt;
    }

    /* Return the amount that was written. */
    return pr.written;
}