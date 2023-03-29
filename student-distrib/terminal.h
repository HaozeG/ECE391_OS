#ifndef TERMINAL_H_
#define TERMINAL_H_
#include "lib.h"

// Initialize terminal
extern int32_t terminal_open();
// Clears terminal specific variables
extern int32_t terminal_close(int32_t fd);
// Read n_bytes char from kbd_buf into buf, return the number of bytes read
extern int32_t terminal_read(int32_t fd, void *buf, int32_t n_bytes);
// Write n_bytes char from buf to screen
extern int32_t terminal_write(int32_t fd, const void *buf, int32_t n_bytes);

#endif
