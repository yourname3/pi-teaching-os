#include <kern/lib.h>

char console_buf[256];

void
readline() {
    uint16_t c;
    int cursor = 0;
    for(;;) {
        /* Repeatedly poll the console in order to read input. */
        // if(!console_poll(&c)) continue;

        /* If we got a raw character, simply append it to the buffer */
        if((c & 0xFF00) == 0) {
            if(c == '\n') {
                /* If we see a newline, we got a line of input */
                // console_putc('\n');
                break;
            }

            console_buf[cursor++] = c;
            // console_putc(c);
        }
    }

    /* Null-terminate the string */
    console_buf[cursor] = 0;
}

void
menu() {
    for(;;) {
        // console_putstr("kern> ");
        readline();
    }
}