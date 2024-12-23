#ifndef K_CONSOLE_H
#define K_CONSOLE_H

// Provides abstractions for a kernel-level "console" that can be hooked up to 
// several concrete sources of input and output.
//
// Motivation: At essentially every level of the kernel, we want access to some 
// kind of text input/output. This lets us show things like kernel panics, and 
// also provide a shell interface to do things with the kernel such as
// starting processes.
//
// However, we don't want to hard-code a single implementation, such as VGA text
// mode, because we want the flexibility of adding new systems as the kernel 
// boots and supporting a wider range of interfaces.
//
// So, the capability to read and write from the terminal is encapsulated into a
// small abstraction, letting the rest of the kernel easily hook up new I/O 
// layers as well as easily make use of the console itself.

#include <kern/types.h>
#include <kern/device.h>

#define CON_ACT_CHAR        0x0000
#define CON_ACT_BACKSPACE   0x0100
#define CON_ACT_LEFT_ARROW  0x0200
#define CON_ACT_RIGHT_ARROW 0x0300
#define CON_ACT_UP_ARROW    0x0400
#define CON_ACT_DOWN_ARROW  0x0500
#define CON_ACT_ENTER       0x0600
#define CON_ACT_INSERT      0x0700
#define CON_ACT_DELETE      0x0800

/**
 * The value returned back to the console driver by a console_io_attach
 * when polled.
 * 
 * Polling has two responsibilities: return inputted character data, e.g.
 * "abcde", and return inputted special keys, such as left arrow, right arrow,
 * etc.
 * 
 * The only particular need for this type is that it can hold at least 16 bits:
 * 8 bits, in the MSB, for the tag (e.g. CON_ACT_BACKSPACE), plus 8 more bits,
 * for the LSB, which store character data in the case of a CON_ACT_CHAR.
 */

DECLARE_DEVICE(console,
    void (*con_putc)(char c);
    bool (*con_poll)(uint16_t *out);

    void (*con_clear)(void);
    void (*con_clear_line)(void);
    void (*con_cursor_left)(int amount);
    void (*con_cursor_right)(int amount);
    void (*con_backspace)(void);
);

#endif