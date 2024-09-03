/**
 * Provides a console_io_attachment for anything that is meant to be redirected
 * to a raw-mode VT100 terminal, such as a UART output.
 */

#include <drivers/generic/con_vt100.h>

static inline void
putc(con_vt100 *vt, char c) {
    vt->vt100_putc(c, vt->user_data);
}

static inline void
putstr(con_vt100 *vt, char *str) {
    while(*str) {
        putc(vt, *str);
        str++;
    }
}

static bool
vt100_init(void *user_data) {
    con_vt100 *vt = user_data;

    if(vt->vt100_init) {
        return vt->vt100_init(vt->user_data);
    }

    return true;
}

static void
vt100_putc(char c, void *user_data) {
    con_vt100 *vt = user_data;

    vt->vt100_putc(c, vt->user_data);
}

static bool
vt100_poll(con_poll_result *out, void *user_data) {
    con_vt100 *vt = user_data;

    if(vt->vt100_poll(&vt->buf[vt->buf_len], user_data)) {
        /* TODO: Parse escape codes, etc */
        *out = CON_ACT_CHAR | vt->buf[0];
        return true;
    }

    return false;
}

static void
vt100_clear(void *user_data) {
    con_vt100 *vt = user_data;

    putstr(vt, "\e[160D\e[0K");
    
}

static void
vt100_clear_line(void *user_data) {

}

static void
vt100_cursor_left(int columns, void *user_data) {

}

static void
vt100_cursor_right(int columns, void *user_data) {

}

static void
vt100_backspace(void *user_data) {

}

void
create_vt100_console_attachment(console_io_attach *io, con_vt100 *vt100) {
    io->con_init = vt100_init;
    io->con_putc = vt100_putc;
    io->con_poll = vt100_poll;
    
    io->con_clear = vt100_clear;
    io->con_clear_line = vt100_clear_line;
    io->con_cursor_left = vt100_cursor_left;
    io->con_cursor_right = vt100_cursor_right;
    io->con_backspace = vt100_backspace;

    /* We track the inner set of functions using the user data pointer. */
    io->user_data = vt100;
}