#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include "types.h"
#include "i8259.h"
#include "lib.h"

#define SCAN_SIZE 255
#define MODES 4

#define KEYPORT 0x60

// special scan code
#define BACKSPACE 0x0E
#define CAPS 0x3A
#define CTRL 0x1D
#define CTRL_REL 0x9D
#define ENTER 0x1C
#define SHIFT_L 0x2A
#define SHIFT_L_REL 0xAA
#define SHIFT_R 0x36
#define SHIFT_R_REL 0xB6
#define TAB 0x0F

#define KEYBOARD_IRQ 1
#define KBDBUF_SIZE 128

extern int enter_buf;
extern int count_char;
extern unsigned char kbd_buffer[KBDBUF_SIZE]; // buffer that holds the characters on terminal line(read by terminal buffer)
extern int caps_buf;
extern int ctrl_buf;
extern int shift_l_buf;
extern int shift_r_buf;
extern int shift_buf;
// extern int full; // whether to allow normal char input

extern void kbd_init();
extern void keyboard_handler();
extern void handle_backspace();

// Initialize terminal
extern int32_t terminal_open();
// Clears terminal specific variables
extern int32_t terminal_close(int32_t fd);
// Read n_bytes char from kbd_buf into buf, return the number of bytes read
extern int32_t terminal_read(int32_t fd, void *buf, int32_t n_bytes);
// Write n_bytes char from buf to screen
extern int32_t terminal_write(int32_t fd, const void *buf, int32_t n_bytes);

#endif
