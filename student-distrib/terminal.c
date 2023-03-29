#include "terminal.h"
#include "lib.h"
#include "keyboard.h"

// --- Terminal ---

int32_t terminal_open()
{
    clear();
    // Clears buf
    count_char = 0;
    // int32_t i;
    // for (i = 0; i < MAX_BUF; i++) {
    //     terminal_buf[i] = 0;
    // }

    return 0;
}

int32_t terminal_close(int32_t fd)
{
    return -1;
}

/*
 * terminal_read
 *   DESCRIPTION: Read from the keyboard buffer into buf. Return if enter is pressed
 *   INPUTS: fd - file descriptor
 *           buf - the buffer that is copied into
 *           n_bytes - maximum bytes to copy
 *   RETURN VALUE: number of bytes copied(include \n) on success,
 *                 -1 on failure
 *   SIDE EFFECTS: Only returns until enter is pressed; 
 */
int32_t terminal_read(int32_t fd, void *buf, int32_t n_bytes)
{
    int32_t i;
    char *buf_to = (char *)buf;
    if (!buf_to || n_bytes > MAX_BUF || n_bytes < 0)
    {
        return -1;
    }
    cli();
    // Wait until enter pressed
    while (!enter_buf)
    {
        sti();
        // create delay
        for (i = 0; i < 200; i++)
        {
        }
        cli();
    };

    i = 0;
    cli();
    // MAX_BUF: up to 128 characters
    while (i < MAX_BUF && i < count_char && i < n_bytes)
    {
        buf_to[i] = kbd_buffer[i];
        i++;
    }

    // reset enter indicator
    enter_buf = 0;
    count_char = 0;
    sti();
    return i;
}

/*
 * terminal_write
 *   DESCRIPTION: Write from buf to screen.
 *   INPUTS: fd - file descriptor
 *           buf - the buffer that is write from
 *           n_bytes - number of bytes to write
 *   RETURN VALUE: number of bytes written(include '\n') on success, -1 on failure
 *   SIDE EFFECTS: Change content on screen
 */
int32_t terminal_write(int32_t fd, const void *buf, int32_t n_bytes)
{
    int32_t i;
    char *buf_from = (char *)buf;
    if (!buf_from || n_bytes > MAX_BUF || n_bytes < 0)
    {
        return -1;
    }

    i = 0;
    cli();
    if (buf_from[i] == '\n') {
        putc('\n');
    }
    while (i < MAX_BUF && i < n_bytes && buf_from[i] != '\n')
    {
        putc(buf_from[i]);
        i++;
        if (buf_from[i] == '\n') {
            putc('\n');
        }
    }

    sti();
    return i;
}
