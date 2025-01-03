#include "printf.h"

#include <string.h>

#define PRINTF_RUNNER_BUFSIZE 32

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

    size_t field_width;
    size_t precision;

    bool in_precision;
    bool zero_pad;
    bool right_pad;

    size_t written;

    /**
     * Buffer the output that we are creating in printf. This allows us to call
     * the printf_write_fn with more data, which should result in less function
     * call overhead.
     */
    char buf[PRINTF_RUNNER_BUFSIZE];
    size_t buflen;
};

/**
 * Writes whatever is in the pr->buf out. Necessary when the buffer is full
 * or when we reach the end of the formatting process.
 */
static void
printf_flush(struct printf_runner *pr) {
    pr->buf[pr->buflen] = '\0';
    pr->write(pr->buf, pr->buflen, pr->userdata);
    pr->written += pr->buflen;
    pr->buflen = 0;
}

/**
 * Push a single char to the internal buffer and, if the buffer is full, empty
 * the buffer (calling pr->write).
 */
static void
printf_push_char(struct printf_runner *pr, char c) {
    pr->buf[pr->buflen++] = c;

    /* The buffer is full when there is only room for a NUL terminator left
     * in the buffer. */
    if(pr->buflen + 1 >= PRINTF_RUNNER_BUFSIZE) {
        printf_flush(pr);
    }
}

static void
printf_do_print(struct printf_runner *pr, const char *prefix1, const char *prefix2, const char *what) {
    size_t prefix1_len = strlen(prefix1);
    size_t prefix2_len = strlen(prefix2);
    size_t what_len = strlen(what);

    size_t total_len = prefix1_len + prefix2_len + what_len;

    /* Compute the total len of the non-padded field. */
    if(what_len < pr->precision) {
        total_len += pr->precision - what_len;
    }

    /* If the total len is less than the field width, pad it with spaces or
     * zeroes depending on the setting. */
    if(total_len < pr->field_width && !pr->right_pad) {
        char pad = pr->zero_pad ? '0' : ' ';
        for(size_t i = 0; i < (pr->field_width - total_len); ++i) {
            printf_push_char(pr, pad);
        }
    }

    for(size_t i = 0; i < prefix1_len; ++i) {
        printf_push_char(pr, prefix1[i]);
    }

    for(size_t i = 0; i < prefix2_len; ++i) {
        printf_push_char(pr, prefix2[i]);
    }

    /* For integer conversions, we print out "precision - what_len" zeroes
     * before the rest of the number. */
    if(what_len < pr->precision) {
        for(size_t i = 0; i < (pr->precision - what_len); ++i) {
            printf_push_char(pr, '0');
        }
    }

    for(size_t i = 0; i < what_len; ++i) {
        printf_push_char(pr, what[i]);
    }

    /* Right pad if needed. */
    if(pr->right_pad) {
        /* Note that right padding is always with spaces. */
        for(size_t i = 0; i < (pr->field_width - total_len); ++i) {
            printf_push_char(pr, ' ');
        }
    }

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

    /* Once value hits 0, we're done.
     * Note that in the case of 0, this loop generates an empty string. With
     * default precision = 1, we nonetheless print out 0. But if the user for
     * some reason sets precision = 0, we will print out nothing--and that
     * is the intended behavior. */
    while(value > 0) {
        /* Back up in the string to "allocate" a char. */
        --at;

        /* Read the current digit from the string. */
        *at = numerals[value % pr->base];

        /* Make the number smaller so we can read the higher digits. */
        value /= pr->base;
    } 
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

    pr->field_width = 0;
    pr->precision = 1;
    pr->in_precision = false;

    pr->zero_pad = false;
    pr->right_pad = false;
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
        printf_push_char(pr, next);
        return;
    }

    /* Now we know we're in_fmt. So, handle all the possible formatting options. */
    switch(next) {
        /* # indicates to print the number out with a base prefix. */
        case '#':
            pr->enable_base_prefix = 1;
            return;

        case '-':
            pr->right_pad = true;
            return;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            if(pr->in_precision) {
                /* If we're computing the precision field, always just multiply
                 * and add (performing string->int conversion). */
                pr->precision *= 10;
                pr->precision += (next - '0');
            }
            else {
                /* If we're not in the precision field, we're in the field width
                 * field.
                 * - If the field width is 0, that means we haven't read any
                 *   digits for it yet. In that case, '0' actually indicates
                 *   to zero pad.
                 * - Otherwise, we simply convert to int as usual. */
                if(pr->field_width == 0 && next == '0') {
                    pr->zero_pad = true;
                }
                else {
                    pr->field_width *= 10;
                    pr->field_width += (next - '0');
                }
            }

            break;
        }

        case '.':
            /* If we see a '.', that means we need to start the precision field.
             * In that case, we replace the default value of 1 with whtaever
             * is in the field, which means we have to intialize pr->precision
             * to 0 so that the logic above will work. */
            pr->precision = 0;
            pr->in_precision = true;
            break;

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
        case 's':
            printf_do_print(pr, "", "", va_arg(pr->va, const char*));
            return;
        default:
            /* For anything that isn't otherwise handled, treat it as essentially
             * '%%' -- i.e. we just print it as a plain character. Note that this
             * means printf will accept formatting specifiers before the % but
             * that's fine. */
            printf_push_char(pr, next);
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

        .buflen = 0,
    };

    va_copy(pr.va, args);

    /* Handle all characters in the format string. */
    for(;;) {
        /* Don't bother handling NUL terminators in step(). */
        if(!*fmt) break;
        printf_step(&pr, *fmt);
        ++fmt;
    }

    /* We must flush the buffer at the end. */
    printf_flush(&pr);

    /* Return the amount that was written. */
    return pr.written;
}