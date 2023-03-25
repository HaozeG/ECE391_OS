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
             '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0',
             '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', '\0',
             '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', '\0', '*', '\0', ' ', '\0'},
            // 10
            {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0',
             '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0',
             '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '\0',
             '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '\0', '*', '\0', ' ', '\0'},
            // 11
            {'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0',
             '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0',
             '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
             '\0', '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0'},
        };

    send_eoi(KEYBOARD_IRQ);
    scan_code = inb(KEYPORT) & 0xff; // read scan code
    if (scan_code == ENTER)
    {
        enter_buf = 1;
        kbd_buffer[count_char] = '\n';
        kbd_buffer[count_char + 1] = '\0';
        count_char += 2;
        putc('\n');
        return;
        // count_char = 0; done by terminal after reading kbd buffer
    }
    else if (scan_code == BACKSPACE)
    {
        handle_backspace();
        return;
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
    else // normal char
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

        if (count_char >= 126 || enter_buf == 1)
            return;

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

    return;
}
