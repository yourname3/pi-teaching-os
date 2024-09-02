#ifndef D_CON_VT100_H
#define D_CON_VT100_H

#include <kern/types.h>
#include <kern/console/console.h>

typedef struct con_vt100 {
    void *user_data;

    bool (*vt100_init)(void *user_data);
    void (*vt100_putc)(char c, void *user_data);
    bool (*vt100_poll)(char *out, void *user_data);

    char buf[16];
    size_t buf_len;
} con_vt100;

void create_vt100_console_attachment(console_io_attach *io, con_vt100 *vt100);

#endif