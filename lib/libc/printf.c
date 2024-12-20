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
    long long base;

    size_t written;
};

static void
printf_do_print(struct printf_runner *pr, const char *prefix, const char *what) {
    size_t prefix_len = strlen(prefix);
    size_t what_len = strlen(what);

    pr->write(prefix, prefix_len, pr->userdata);
    pr->write(what, what_len, pr->userdata);

    pr->written += prefix_len + what_len;

    /* Whenever we do a print, reset the in_fmt flag to false. */
    pr->in_fmt = 0;
}

static const char*
numerals = "0123456789ABCDEF";

static void
printf_do_int(struct printf_runner *pr, long long value) {
    char buf[16] = { 0 };

    /* Initialize 'at' at the end of the string because we decrement it before
     * writing to it, so we'll get that one extra char for the terminator. */
    char *at = &buf[15];

    const char *prefix = "";
    if(value < 0) {
        prefix = "-";
        /* TODO handle MIN_LONG_LONG? */

        value = -value;
    }

    do {
        /* Back up in the string to "allocate" a char. */
        --at;

        /* Read the current digit from the string. */
        *at = numerals[value % pr->base];

        /* Make the number smaller so we can read the higher digits. */
        value /= pr->base;
    } while(value > 0); /* Once value hits 0, we're done. */

    /* Print based on the prefix and the "at" value. */
    printf_do_print(pr, prefix, at);
}

static void
printf_step(struct printf_runner *pr, char next) {
    if(!pr->in_fmt) {
        /* If we're not currently in a format string, there's only two possibilities:
         * we see a %, starting a format string, or we don't, in which case we
         * just print that character. */
        if(next == '%') {
            /* Switch into in_fmt mode */
            pr->in_fmt = 1;
            return;
        }

        /* Print the character */
        char buf[2] = { next, '\0' };
        printf_do_print(pr, "", buf);
        return;
    }

    /* Now we know we're in_fmt. So, handle all the possible formatting options. */
    switch(next) {
        case 'd': pr->base = 10; printf_do_int(pr, va_arg(pr->va, long long)); return;
        default:
            /* For anything that isn't otherwise handled, treat it as essentially
             * '%%' -- i.e. we just print it as a plain character. */
            {
                char buf[2] = { next, '\0' };
                printf_do_print(pr, "", buf);
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