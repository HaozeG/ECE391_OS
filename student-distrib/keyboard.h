#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include "types.h"
#include "i8259.h"
#include "lib.h"
#include "terminal.h"

#define SCAN_SIZE 255
#define MODES 4

#define KEYPORT 0x60

// special scan code
#define ESC 0x01
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
#define EXT 0xE0    // extended byte
#define ALT 0x38
#define ALT_REL 0xB8
#define EXT_CURSOR_UP 0x48
#define EXT_CURSOR_UP_REL 0xC8
#define EXT_CURSOR_DOWN 0x50
#define EXT_CURSOR_DOWN_REL 0xD0
#define EXT_CURSOR_LEFT 0x4B
#define EXT_CURSOR_LEFT_REL 0xCB
#define EXT_CURSOR_RIGHT 0x4D
#define EXT_CURSOR_RIGHT_REL 0xCD
#define F1 0x3B
#define F1_REL 0xBB
#define F2 0x3C
#define F2_REL 0xBC
#define F3 0x3D
#define F3_REL 0xBD

#define KEYBOARD_IRQ 1
#define KBDBUF_SIZE 128

#define MAX_BUF 128

extern int enter_buf[NUM_TERM];
extern int count_char[NUM_TERM];
extern unsigned char kbd_buffer[NUM_TERM][KBDBUF_SIZE]; // buffer that holds the characters on terminal line(read by terminal buffer)
extern int caps_buf;
extern int ctrl_buf;
extern int shift_l_buf;
extern int shift_r_buf;
extern int shift_buf;
extern int alt_buf;
extern int cursor_l_buf;
extern int cursor_r_buf;
extern int cursor_u_buf;
extern int cursor_d_buf;
// extern int display_term;
// extern int full; // whether to allow normal char input

extern void kbd_init();
extern void keyboard_handler();
extern void handle_backspace();

#endif
