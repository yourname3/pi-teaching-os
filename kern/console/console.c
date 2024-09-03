#include <kern/console/console.h>

static console_io_attach *console_root = NULL;

void
console_attach(console_io_attach *attachment) {
    /* If the attachment can't initialize, don't attach it. */
    if(!attachment->con_init(attachment->user_data)) {
        return;
    }

    /* Add the attachment to the linked list. */
    attachment->next = console_root;
    console_root = attachment;
}

void
console_putc(char c) {
    console_io_attach *con = console_root;
    while(con) {
        con->con_putc(c, con->user_data);
        con = con->next;
    }
}

void
console_putstr(char *str) {
    while(*str) {
        console_putc(*str);
        str++;
    }
}

bool
console_getc(char* out) {
    console_io_attach *con = console_root;
    con_poll_result poll;
    while(con) {
        if(con->con_poll(&poll, con->user_data)) {
            if((poll & 0xFF00) == 0x0000) {
                *out = (poll & 0xFF);
                return true;
            }
        }
    }
    return false;
}