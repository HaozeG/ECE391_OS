#include "mouse.h"
#include "kalman_filter.h"
#include "kalman_filter.h"

// global variables for mouse
mouse_t mouse;
kalman1_state mouse_state_x;
kalman1_state mouse_state_y;kalman1_state mouse_state_x;
kalman1_state mouse_state_y;

void mouse_init()
{
    // enable mouse
    wait_for_write();
    outb(ENABLE_MOUSE, MOUSESTATUS_PORT);

    // modify compaq status
    wait_for_write();
    outb(GET_COMPAQ_STATUS, MOUSESTATUS_PORT);
    wait_for_read();
    uint8_t status = inb(MOUSEIO_PORT);
    status |= 0x02; // set bit 1, enable IRQ12
    status &= 0xDF; // clear bit 5, disable mouse clock
    wait_for_write();
    outb(SET_COMPAQ_STATUS, MOUSESTATUS_PORT);
    wait_for_write();
    outb(status, MOUSEIO_PORT);

    // set default mouse(Disables streaming, sets the packet rate to 100 per second, and resolution to 4 pixels per mm)
    set_mouse_cmd(CMD_SET_DEFAULTS);
    // enable streaming
    wait_ack();
    set_mouse_cmd(CMD_ENABLE_STREAM);
    wait_ack();
    // set sample rate
    set_mouse_cmd(CMD_SET_SAMPLERATE);
    wait_ack();
    wait_for_write();
    outb(200, MOUSEIO_PORT);
    // set scaling to 1:1
    set_mouse_cmd(CMD_SET_SAMPLERATE);
    wait_ack();
    // enable IRQ
    enable_irq(MOUSE_IRQ);
    mouse.mouse_l_click = 0;
    mouse.mouse_r_click = 0;
    mouse.mouse_m_click = 0;
    mouse.mouse_x = IMAGE_X_DIM / 2;
    mouse.mouse_y = IMAGE_Y_DIM / 2;

    kalman1_init(&mouse_state_x, 0, 0.001);
    kalman1_init(&mouse_state_y, 0, 0.001);
}

void mouse_handler()
{
    // return ;
    send_eoi(MOUSE_IRQ);
    // wait_for_read();
    uint8_t status = inb(MOUSEIO_PORT);
    // check if the status is valid
    if (!(status & 0x08) == 0x08 || (status & 0x80) == 0x80 || (status & 0x40) == 0x40) // check x,y overflow bit and always 1 bit
    {
        return;
    }
    // printf("status: %x\n", status);

    int signX = (status & (1 << 4)) ? 1 : 0;
    int signY = (status & (1 << 5)) ? 1 : 0;

    // get x,y movement
    // wait_for_read();
    int movex = inb(MOUSEIO_PORT);
    // wait_for_read();
    int movey = inb(MOUSEIO_PORT);
    if (signX)
    {
        movex |= 0xFFFFFF00;
    }
    if (signY)
    {
        movey |= 0xFFFFFF00;
    }

    // get mouse button status
    mouse.mouse_l_click = (status & 0x01) ? 1 : 0;
    mouse.mouse_r_click = (status & 0x02) ? 1 : 0;
    mouse.mouse_m_click = (status & 0x04) ? 1 : 0;

    // update mouse position
    mouse.mouse_x = kalman1_filter(&mouse_state_x, movex);
    mouse.mouse_y = -kalman1_filter(&mouse_state_y, movey);
    // mouse.mouse_x = movex;
    // mouse.mouse_y = -movey;
    // printf("x: %d, y: %d\n", mouse.mouse_x, mouse.mouse_y);
    return;
}

void set_mouse_cmd(uint8_t cmd)
{
    wait_for_write();
    outb(0xD4, MOUSESTATUS_PORT);
    wait_for_write();
    outb(cmd, MOUSEIO_PORT);
}

/*
 * wait_for_read
 *   DESCRIPTION: Waits for the mouse controller's output buffer to be ready for reading.
 *   INPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Blocks the execution until the Output Buffer Full (OBF) flag is set,
 *                 indicating that data is available in the output buffer for reading.
 */
void wait_for_read()
{
    uint8_t status;
    int t = 10000;
    do
    {
        status = inb(MOUSESTATUS_PORT);
    } while (t-- && !(status & 0x01));
}

void wait_for_write()
{
    uint8_t status;
    int t = 10000;
    do
    {
        status = inb(MOUSESTATUS_PORT);
    } while (t-- && status & 0x02);
}

void wait_ack()
{
    uint8_t data;
    int t = 10000;
    do
    {
        data = inb(MOUSEIO_PORT);
    } while (t-- && data != 0xFA);
}


int mouse_open()
{

    return 0;
}

int mouse_close(int32_t fd)
{
    return 0;
}

int mouse_write(int32_t fd, const void* buf, int32_t nbytes)
{
    if (!buf || fd < 0 || fd > 7)
    {
        return -1;
    }
    return 0;
}

int mouse_read(int32_t fd, void* buf, int32_t nbytes)
{
    if (!buf || fd < 0 || fd > 7)
    {
        return -1;
    }
    sti();

    // int i = 10000;
    // while (i--)
    // {
    // };

    mouse_t *mouse_buf = (mouse_t *)buf;
    mouse_buf->mouse_l_click = mouse.mouse_l_click;
    mouse_buf->mouse_r_click = mouse.mouse_r_click;
    mouse_buf->mouse_m_click = mouse.mouse_m_click;
    mouse_buf->mouse_x = mouse.mouse_x;
    mouse_buf->mouse_y = mouse.mouse_y;
    mouse.mouse_x = 0;
    mouse.mouse_y = 0;
    mouse.mouse_l_click = 0;
    mouse.mouse_r_click = 0;
    mouse.mouse_m_click = 0;
    return 0;
}
