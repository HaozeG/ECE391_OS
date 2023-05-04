#ifndef MOUSE_H_
#define MOUSE_H_

#include "types.h"
#include "lib.h"
#include "terminal.h"
#include "i8259.h"
#include "vga.h"

#define MOUSEIO_PORT 0x60
#define MOUSESTATUS_PORT 0x64

#define MOUSE_IRQ 12

// command bytes
#define ENABLE_MOUSE 0xA8
#define GET_COMPAQ_STATUS 0x20
#define SET_COMPAQ_STATUS 0x60
#define PRE_CMD 0xD4
#define CMD_RESET 0xFF
#define CMD_RESEND_PACKET 0xFE
#define CMD_SET_DEFAULTS 0xF6
#define CMD_ENABLE_STREAM 0xF4
#define CMD_SET_SAMPLERATE 0xF3
#define CMD_GET_STATUS 0xE9
#define CURSOR_X_DIM 12
#define CURSOR_Y_DIM 12

extern void mouse_init();
extern void mouse_handler();
extern void set_mouse_cmd(uint8_t cmd);
extern void wait_ack();
extern void wait_for_write();
extern void wait_for_read();
extern void update_cursor(int x, int y);

extern int mouse_open();
extern int mouse_close(int32_t fd);
extern int mouse_read(int32_t fd, void *buf, int32_t n);
extern int mouse_write(int32_t fd, const void *buf, int32_t n);

typedef struct
{
    int mouse_x;
    int mouse_y;
    int mouse_l_click;
    int mouse_r_click;
    int mouse_m_click;
} mouse_t;

#endif
