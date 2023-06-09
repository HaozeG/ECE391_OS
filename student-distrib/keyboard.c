#include "keyboard.h"
#include "syscall.h"
#include "lib.h"
#include "terminal.h"
#include "scheduler.h"
#include "vga.h"

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
// int full = 0; // whether to allow normal char input

int count_char[NUM_TERM] = {0, 0, 0}; // valid byte numbers in kbd buffer
int enter_buf[NUM_TERM] = {0, 0, 0};
unsigned char kbd_buffer[NUM_TERM][KBDBUF_SIZE];

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

    scan_code = inb(KEYPORT) & 0xff; // read scan code
    send_eoi(KEYBOARD_IRQ);

    if (shift_l_buf == 1 || shift_r_buf == 1)
        shift_buf = 1;
    else
        shift_buf = 0;

    int mode = caps_buf * 2 + shift_buf;
    ascii = scancode_table[mode][scan_code];
    if (extended) {
        switch (scan_code)
        {
        case ESC:
            if (is_mode_X) {
                kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                count_char[display_term]++;
            } 
            break;    
        case ALT:
            alt_buf = 1;
            break;
        case ALT_REL:
            alt_buf = 0;
            break;
        case EXT_CURSOR_UP:
            cursor_u_buf = 1;
            if (is_mode_X) {
                kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                count_char[display_term]++;
            } 
            break;
        case EXT_CURSOR_UP_REL:
            cursor_u_buf = 0;
            break;
        case EXT_CURSOR_DOWN:
            cursor_d_buf = 1;
            if (is_mode_X) {
                kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                count_char[display_term]++;
            } 
            break;
        case EXT_CURSOR_DOWN_REL:
            cursor_d_buf = 0;
            break;
        case EXT_CURSOR_LEFT:
            cursor_l_buf = 1;
            if (is_mode_X) {
                kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                count_char[display_term]++;
            } 
            break;
        case EXT_CURSOR_LEFT_REL:
            cursor_l_buf = 0;
            break;
        case EXT_CURSOR_RIGHT:
            cursor_r_buf = 1;
            if (is_mode_X) {
                kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                count_char[display_term]++;
            } 
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
            if (is_mode_X) {
                kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                count_char[display_term]++;
            } else {
                schedule_disable = 1;
                enter_buf[display_term] = 1;
                if (count_char[display_term] < MAX_BUF) {
                    kbd_buffer[display_term][count_char[display_term]] = '\n';
                    count_char[display_term]++;
                    putc_display('\n');
                }
                schedule_disable = 0;
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
        case ESC:
            if (is_mode_X) {
                kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                count_char[display_term]++;
            } 
            break; 
        // use keypad as cursor
        case EXT_CURSOR_UP:
            cursor_u_buf = 1;
            if (is_mode_X) {
                kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                count_char[display_term]++;
            } 
            break;
        case EXT_CURSOR_UP_REL:
            cursor_u_buf = 0;
            break;
        case EXT_CURSOR_DOWN:
            cursor_d_buf = 1;
            if (is_mode_X) {
                kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                count_char[display_term]++;
            } 
            break;
        case EXT_CURSOR_DOWN_REL:
            cursor_d_buf = 0;
            break;
        case EXT_CURSOR_LEFT:
            cursor_l_buf = 1;
            if (is_mode_X) {
                kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                count_char[display_term]++;
            } 
            break;
        case EXT_CURSOR_LEFT_REL:
            cursor_l_buf = 0;
            break;
        case EXT_CURSOR_RIGHT:
            cursor_r_buf = 1;
            if (is_mode_X) {
                kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                count_char[display_term]++;
            } 
            break;
        case EXT_CURSOR_RIGHT_REL:
            cursor_r_buf = 0;
            break;
        case EXT:
            extended = 1;
            break;
        default:
            if (alt_buf) {
                if (!is_mode_X) {
                    if (scan_code >= F1 && scan_code <= F3) {
                        if (display_term != (scan_code - F1)) {
                            // switch terminal
                            terminal_switch(scan_code - F1);
                        }
                        return;
                    }
                }
                return;
            }
            if (ctrl_buf) // CTRL key combinations
            {
                if (ascii == 'l' || ascii == 'L') // CTRL + L: clear the screen
                {
                    if (!is_mode_X) {
                        schedule_disable = 1;
                        clear_display();
                        schedule_disable = 0;
                        // kbd_buffer[] = {'\0'};
                        count_char[display_term] = 0;
                    }
                    return;
                } else if (ascii == 'c' || ascii == 'C') {
                    // TODO: halt the visible one
                    // schedule_disable = 1;
                    // while (running_term != display_term){
                    //     schedule_disable = 0;
                    //     schedule_disable = 1;
                    // }
                    // sys_halt(0);
                    // // May change this implementation to signal
                    // // terminal_switch(display_term);
                    // // sys_halt(0);
                    return;
                } else {
                    return;
                }
            }

            // backspace
            if (scan_code == BACKSPACE)
            {
                if (is_mode_X) {
                    kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                    count_char[display_term]++;
                } else {
                    schedule_disable = 1;
                    handle_backspace();
                    schedule_disable = 0;
                }
                return;
            }

            if (count_char[display_term] >= (MAX_BUF - 1) || enter_buf[display_term] == 1)
                return;

            // tab
            if (scan_code == TAB)
            {
                if (is_mode_X) {
                    kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                    count_char[display_term]++;
                } else {
                    schedule_disable = 1;
                    // putc_display('\t');
                    int i;
                    for (i = 0; i < 4; i++)
                    {
                        putc_display(' ');
                    }
                    kbd_buffer[display_term][count_char[display_term]] = '\t';
                    count_char[display_term]++;
                    schedule_disable = 0;
                }
                return;
            }

            if (ascii != 0)
            {
                if (is_mode_X) {
                    kbd_buffer[display_term][count_char[display_term]] = scan_code; // write to kbd buffer
                    count_char[display_term]++;
                } else {
                    // printf("mode = %d, shift = %d, count = %d\n", mode,shift_buf,count_char);
                    schedule_disable = 1;
                    putc_display(ascii);                    // write char to screen
                    kbd_buffer[display_term][count_char[display_term]] = ascii; // write to kbd buffer
                    count_char[display_term]++;
                    schedule_disable = 0;
                } 
            }
            return;
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
    if (count_char[display_term] == 0)
    {
        return;
    }
    putc_display('\b');    
    count_char[display_term]--;
    // printf("%d", count_char);
    return;
}

