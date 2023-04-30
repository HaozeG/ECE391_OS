#ifndef _VGA_H
#define _VGA_H

#include "lib.h"

typedef struct img_t
{
    int32_t x;
    int32_t y;
    int32_t dim_x;
    int32_t dim_y;
    int8_t preserve_mask;
    int8_t *ptr;
} img_t;

/* 
 * IMAGE  is the whole screen in mode X: 320x200 pixels in our flavor.
 * SCROLL is the scrolling region of the screen.
 *
 * X_DIM   is a horizontal screen dimension in pixels.
 * X_WIDTH is a horizontal screen dimension in 'natural' units
 *         (addresses, characters of text, etc.)
 * Y_DIM   is a vertical screen dimension in pixels.
 */
#define FONT_HEIGHT 16
#define FONT_WIDTH 8
#define IMAGE_X_DIM     320   /* pixels; must be divisible by 4             */
#define IMAGE_Y_DIM     200   /* pixels                                     */
#define IMAGE_X_WIDTH   (IMAGE_X_DIM / 4)          /* addresses (bytes)     */
#define SCROLL_X_DIM    IMAGE_X_DIM                /* upper screen size     */
#define SCROLL_Y_DIM    IMAGE_Y_DIM                 /* upper screen size*/
#define SCROLL_X_WIDTH  (IMAGE_X_DIM / 4)          /* addresses (bytes)     */

#define STATUS_BAR_HEIGHT       (FONT_HEIGHT + 2)
#define STATUS_BAR_MAX          40      // SCROLL_SIZE / FONT_WIDTH
#define STATUS_BAR_SIZE         1440    // 320 * STATUS_BAR_HEIGHT / 4 bytes

#define SCROLL_SIZE             (SCROLL_X_WIDTH * SCROLL_Y_DIM)
#define SCREEN_SIZE             (SCROLL_SIZE * 4 + 1)
#define BUILD_BUF_SIZE          (SCREEN_SIZE + 20000)
#define BUILD_BASE_INIT         ((BUILD_BUF_SIZE - SCREEN_SIZE) / 2)

#define COLOR_TRANSPARENT 0xFF

extern int draw_x, draw_y;
extern int is_mode_X;

/* configure VGA for mode X; initializes logical view to (0,0) */
extern int set_mode_X();

/* return to text mode */
extern void clear_mode_X();

/* set logical view window coordinates */
extern void set_view_window(int scr_x, int scr_y);

/* show the logical view window on the monitor */
extern void show_screen();

/* clear the video memory in mode X */
extern void clear_screens();

/*
 * draw a 12x12 block with upper left corner at logical position
 * (pos_x,pos_y); any part of the block outside of the logical view window
 * is clipped (cut off and not drawn)
 */
extern void draw_full_block(int pos_x, int pos_y, unsigned char* blk);

/* draw a horizontal line at vertical pixel y within the logical view window */
extern int draw_horiz_line(int y);

/* draw a vertical line at horizontal pixel x within the logical view window */
extern int draw_vert_line(int x);

/* configure VGA for split screen(status bar) with STATUS_BAR_HEIGHT for lower screen */
extern void set_status_bar();

/* update the content in status bar based on input info */
// extern void update_status_bar(int level, int num_fruit, int sec_elapsed);

// extern void draw_player_masked(int player_x, int player_y, unsigned char *player_mask, unsigned char *player_block);
// extern void undraw_player_masked(int player_x, int player_y, unsigned char *player_mask, unsigned char *player_block);

// set the original and semi-transparent palette color
// extern void set_color_wall(int level);
// extern void set_color_player(int ticks);

// draw a block of semi-transparent text
extern void draw_floating_text(int pos_x, int pos_y, int type);
extern void undraw_floating_text(int pos_x, int pos_y);

// used as system call
extern int32_t vga_open();
extern int32_t vga_close(int32_t fd);
extern int32_t vga_read(int32_t fd, void *buf, int32_t n);
extern int32_t vga_write(int32_t fd, void *buf);

#endif
