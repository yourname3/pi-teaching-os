/**
 * Provides a console_io_attachment for anything that is meant to be redirected
 * to a raw-mode VT100 terminal, such as a UART output.
 */

#include <drivers/generic/con_vt100.h>

#include <kern/devices/console.h>

static inline void
putc(char c) {
    con_vt100_dev->putc(c);
}

static inline void
putstr(char *str) {
    while(*str) {
        putc(*str);
        str++;
    }
}

static bool
vt100_poll(uint16_t *out) {
    char buf;

    if(con_vt100_dev->poll(&buf)) {
        /* TODO: Parse escape codes, etc. Will require a buffer..? */
        *out = CON_ACT_CHAR | buf;
        return true;
    }

    return false;
}

static void
vt100_clear() {
    putstr("\e[160D\e[0K");
}

static void
vt100_clear_line() {

}

static void
vt100_cursor_left(int columns) {

}

static void
vt100_cursor_right(int columns) {

}

static void
vt100_backspace() {

}

IMPL_DEVICE(console, con_vt100,
    .con_putc = putc,
    .con_poll = vt100_poll,

    .con_clear = vt100_clear,
    .con_clear_line = vt100_clear_line,
    .con_cursor_left = vt100_cursor_left,
    .con_cursor_right = vt100_cursor_right,
    .con_backspace = vt100_backspace
);

DEFINE_DEVICE(con_vt100);

void
con_vt100_init() {
    console_dev = &con_vt100;
}