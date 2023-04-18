#include "keyboard.h"
#include "syscall.h"
#include "lib.h"

int caps_buf = 0;
int ctrl_buf = 0;
int shift_l_buf = 0;
int shift_r_buf = 0;
int shift_buf = 0;
int extended = 0;
int alt_buf = 0;
int cursor_l_buf = 0;
int cursor_r_buf = 0;
int cursor_u_buf = 0;
int cursor_d_buf = 0;
int alt_F_buf = 0;
// int full = 0; // whether to allow normal char input

int count_char = 0; // valid byte numbers in kbd buffer
int enter_buf = 0;
unsigned char kbd_buffer[] = {'\0'};

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

    cli();
    scan_code = inb(KEYPORT) & 0xff; // read scan code
    scan_code = inb(KEYPORT) & 0xff; // read scan code
    send_eoi(KEYBOARD_IRQ);
    sti();
    if (shift_l_buf == 1 || shift_r_buf == 1)
        shift_buf = 1;
    else
        shift_buf = 0;

    int mode = caps_buf * 2 + shift_buf;
    ascii = scancode_table[mode][scan_code];
    if (extended) {
        switch (scan_code)
        {
        case ALT:
            alt_buf = 1;
            break;
        case ALT_REL:
            alt_buf = 0;
            break;
        case EXT_CURSOR_UP:
            cursor_u_buf = 1;
            break;
        case EXT_CURSOR_UP_REL:
            cursor_u_buf = 0;
            break;
        case EXT_CURSOR_DOWN:
            cursor_d_buf = 1;
            break;
        case EXT_CURSOR_DOWN_REL:
            cursor_d_buf = 0;
            break;
        case EXT_CURSOR_LEFT:
            cursor_l_buf = 1;
            break;
        case EXT_CURSOR_LEFT_REL:
            cursor_l_buf = 0;
            break;
        case EXT_CURSOR_RIGHT:
            cursor_r_buf = 1;
            break;
        case EXT_CURSOR_RIGHT_REL:
            cursor_r_buf = 0;
            break;
        default:
            break;
        }
        extended = 0;
    } else {
        switch (scan_code)
        {
        // func keys
        case ENTER:
            enter_buf = 1;
            if (count_char < MAX_BUF) {
                kbd_buffer[count_char] = '\n';
                count_char++;
                putc('\n');
            }
            break;
        case CAPS:
            caps_buf = caps_buf ^ 0x01;
            break;
        case CTRL:
            ctrl_buf = 1;
            break;
        case CTRL_REL:
            ctrl_buf = 0;
            break;
        case SHIFT_L:
            shift_l_buf = 1;
            break;
        case SHIFT_L_REL:
            shift_l_buf = 0;
            break;
        case SHIFT_R:
            shift_r_buf = 1;
            break;
        case SHIFT_R_REL:
            shift_r_buf = 0;
            break;
        case ALT:
            alt_buf = 1;
            break;
        case ALT_REL:
            alt_buf = 0;
            break;
        // use keypad as cursor
        // case EXT_CURSOR_UP:
        //     cursor_u_buf = 1;
        //     cursor_update(screen_x, --screen_y);
        //     break;
        // case EXT_CURSOR_UP_REL:
        //     cursor_u_buf = 0;
        //     break;
        // case EXT_CURSOR_DOWN:
        //     cursor_d_buf = 1;
        //     cursor_update(screen_x, ++screen_y);
        //     break;
        // case EXT_CURSOR_DOWN_REL:
        //     cursor_d_buf = 0;
        //     break;
        // case EXT_CURSOR_LEFT:
        //     cursor_l_buf = 1;
        //     cursor_update(--screen_x, screen_y);
        //     break;
        // case EXT_CURSOR_LEFT_REL:
        //     cursor_l_buf = 0;
        //     break;
        // case EXT_CURSOR_RIGHT:
        //     cursor_r_buf = 1;
        //     cursor_update(++screen_x, screen_y);
        //     break;
        // case EXT_CURSOR_RIGHT_REL:
        //     cursor_r_buf = 0;
        //     break;
        case EXT:
            extended = 1;
            break;
        default:
            if (alt_buf) {
                if (scan_code >= F1 && scan_code <= F3) {
                    if (alt_F_buf != (scan_code - F1 + 1)) {
                        alt_F_buf = scan_code - F1 + 1;
                        // switch terminal
                        
                    }
                    return;
                }
                return;
            }
            if (ctrl_buf) // CTRL key combinations
            {
                if (ascii == 'l' || ascii == 'L') // CTRL + L: clear the screen
                {
                    clear();
                    // kbd_buffer[] = {'\0'};
                    count_char = 0;

                    return;
                }
                if (ascii == 'c' || ascii == 'C')
                {
                    count_char = 0;
                    sys_halt(0);
                }
                return;
            }

            // backspace
            if (scan_code == BACKSPACE)
            {
                handle_backspace();
                return;
            }

            if (count_char >= (MAX_BUF - 1) || enter_buf == 1)
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
            break;
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

