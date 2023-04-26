#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

typedef struct img_t
{
    int32_t x;
    int32_t y;
    int32_t dim_x;
    int32_t dim_y;
    int8_t *ptr;
} img_t;


int32_t fd_vga, fd_img, fd_rtc;

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

// used as the base for drawing
int draw_x = 0;
int draw_y = 0;

int main()
{
    int32_t cnt;
    uint8_t buf[IMAGE_X_DIM *IMAGE_Y_DIM + 1];
    img_t UI, palette;
    int pixel_size = 6;
    fd_vga = ece391_open((uint8_t *)"vga");
    // Output background
    fd_img = ece391_open((uint8_t *)"UI.txt");
    if (IMAGE_X_DIM * IMAGE_Y_DIM == (cnt = ece391_read(fd_img, (void *)buf, IMAGE_X_DIM *IMAGE_Y_DIM))) {
        UI.dim_x = IMAGE_X_DIM;
        UI.dim_y = IMAGE_Y_DIM;
        UI.x = 0;
        UI.y = 0;
        UI.ptr = (int8_t *)buf;
        ece391_write(fd_vga, (void *)&UI, 0);
    }
    ece391_close(fd_img);

    // show palette
    palette.x = 120;
    palette.y = 20;
    palette.dim_x = 8;
    palette.dim_y = 256;
    palette.ptr = (int8_t *)buf;
    int i;
    for (i = 0; i < palette.dim_x * palette.dim_y; i++) {
        buf[i] = (uint8_t)(i / palette.dim_x / 4);
    }
    buf[i] = 0;
    ece391_write(fd_vga, (void *)&palette, 0);

    while (1) {};    
    return 0;
}
