#include "interrupt.h"

int caps_buf = 0;
int ctrl_buf = 0;
int shift_l_buf = 0;
int shift_r_buf = 0;
int shift_buf = 0;
// int full = 0; // whether to allow normal char input

int count_char = 0; // valid byte numbers in kbd buffer
int enter_buf = 0;
unsigned char kbd_buffer[] = {'\0'};

#define MAX_BUF 128
/*
 * keyboard_handler
 *   DESCRIPTION: Handle keyboard interrupts, display pressed key to screen
 *                , allowing shift, CapsLock. It will clear the screen with
 *                CTRL + L combination.
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void keyboard_handler()
{
    unsigned char scan_code;
    unsigned char ascii;

    // array mapping scan code to ascii characters
    unsigned char scancode_table[MODES][SCAN_SIZE] =
        {
            // output modes depend on 2 bits: caps and shift
            // 00 (default)
            {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0',
             '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0',
             '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0',
             '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', '*', '\0', ' ', '\0'},
            // 01
            {'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0',
             '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0',
             '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', '\0',
             '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0'},

            // 10
            {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0',
             '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0',
             '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '\0',
             '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '\0', '*', '\0', ' ', '\0'},
            // 11
            {'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0',
             '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0',
             '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', '\0',
             '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', '\0', '*', '\0', ' ', '\0'}};

    send_eoi(KEYBOARD_IRQ);
    scan_code = inb(KEYPORT) & 0xff; // read scan code
    if (scan_code == ENTER)
    {
        enter_buf = 1;
        kbd_buffer[count_char] = '\n';
        count_char++;
        putc('\n');
        return;
        // count_char = 0; done by terminal after reading kbd buffer
    }
    else if (scan_code == CAPS)
    {
        caps_buf = caps_buf ^ 0x01;
        return;
    }
    else if (scan_code == CTRL)
    {
        ctrl_buf = 1;
        return;
    }
    else if (scan_code == CTRL_REL)
    {
        ctrl_buf = 0;
        return;
    }
    else if (scan_code == SHIFT_L)
    {
        shift_l_buf = 1;
        return;
    }
    else if (scan_code == SHIFT_L_REL)
    {
        shift_l_buf = 0;
        return;
    }
    else if (scan_code == SHIFT_R)
    {
        shift_r_buf = 1;
        return;
    }
    else if (scan_code == SHIFT_R_REL)
    {
        shift_r_buf = 0;
        return;
    }
    else // normal char or tab or backspace
    {
        if (shift_l_buf == 1 || shift_r_buf == 1)
            shift_buf = 1;
        else
            shift_buf = 0;

        int mode = caps_buf * 2 + shift_buf;
        ascii = scancode_table[mode][scan_code];

        if (ctrl_buf) // CTRL key combinations
        {
            if (ascii == 'l' || ascii == 'L') // CTRL + L: clear the screen
            {
                clear();
                // kbd_buffer[] = {'\0'};
                count_char = 0;

                return;
            }
            return;
        }

        // backspace
        if (scan_code == BACKSPACE)
        {
            handle_backspace();
            return;
        }

        if (count_char >= (MAX_BUF-1) || enter_buf == 1)
            return;

        // tab
        if (scan_code == TAB)
        {
            putc('\t');
            kbd_buffer[count_char] = '\t';
            count_char++;
            return;
        }

        if (ascii != 0)
        {
            // printf("mode = %d, shift = %d, count = %d\n", mode,shift_buf,count_char);
            putc(ascii);                    // write char to screen
            kbd_buffer[count_char] = ascii; // write to kbd buffer
            count_char++;
        }
    }
}

/*
 * handle_backspace
 *   DESCRIPTION: Handle backspace key press, to delete one
 *                character on the screen
 *   RETURN VALUE: none
 *   SIDE EFFECTS: delete the last char in kbd buffer
 */
void handle_backspace()
{
    if (count_char == 0)
    {
        return;
    }
    putc('\b');
    count_char--;
    // printf("%d", count_char);
    return;
}

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
    return 0;
}

/*
 * terminal_read
 *   DESCRIPTION: Read from the keyboard buffer into buf. Return if enter is pressed
 *   INPUTS: fd - file descriptor
 *           buf - the buffer that is copied into
 *           n_bytes - maximum bytes to copy
 *   RETURN VALUE: number of bytes copied(include \n) on success,
 *                 -1 on failure
 *   SIDE EFFECTS: Only returns until enter is pressed; last char in buf is '\n'
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
    // MAX_BUF - 1: up to 127 characters
    while (i < (MAX_BUF - 1) && kbd_buffer[i] != '\n' && i < n_bytes)
    {
        buf_to[i] = kbd_buffer[i];
        i++;
    }
    // buf_to[i] = '\n';
    // i++;

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
 *   RETURN VALUE: number of bytes written(does not include '\n') on success, -1 on failure
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
    while (i < n_bytes && buf_from[i] != '\n')
    {
        putc(buf_from[i]);
        i++;
    }
    // putc('\n');
    // i++;

    sti();
    return i;
}
