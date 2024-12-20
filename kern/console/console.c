#include <kern/console/console.h>

static console_io_attach *console_root = NULL;

void
console_attach(console_io_attach *attachment) {
    if(!attachment->con_init) {
        attachment->con_init = dummy_init;
    }

    /* If the attachment can't initialize, don't attach it. */
    if(!attachment->con_init(attachment->user_data)) {
        return;
    }

    if(!attachment->con_poll) {
        attachment->con_poll = dummy_poll;
    }
    if(!attachment->con_putc) {
        attachment->con_putc = dummy_putc;
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
console_putstr(const char *str) {
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

bool
console_poll(con_poll_result *out) {
    console_io_attach *con = console_root;
    con_poll_result poll;

    while(con) {
        if(con->con_poll(&poll, con->user_data)) {
            if(out) *out = poll;
            return true;
        }
    }
    return false;
}

bool dummy_init(void *user_data) { return true; }
void dummy_putc(char c, void *user_data) { }
bool dummy_poll(con_poll_result *out, void *user_data) { return false; }