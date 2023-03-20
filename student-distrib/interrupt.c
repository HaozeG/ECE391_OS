#include "interrupt.h"
#include "lib.h"

#define SCAN_SIZE 255
#define PRESS_MAX 0x57
#define KEYPORT 0x60
#define ASC_a 0x61
#define ASC_z 0x7A
#define ASC_0 0x30
#define ASC_9 0x39

#define KEYBOARD_IRQ 1

/*
 * keyboard_handler
 *   DESCRIPTION: Handle keyboard interrupts. When number or letter key is
 *                pressed, output this character(lower case) to screen
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void keyboard_handler()
{
    unsigned char scan_code;
    unsigned char ascii;
    // array mapping scan code to ascii characters
    unsigned char scancode_table[SCAN_SIZE] =
        {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
         'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0', '\0',
         'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\',
         'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', '*', '\0', ' ', '\0'};
    send_eoi(KEYBOARD_IRQ);
    scan_code = inb(KEYPORT) & 0xff; // read scan code
    if (scan_code > PRESS_MAX)       // only display when key is pressed
        return;

    ascii = scancode_table[scan_code];
    if ((ascii >= ASC_a && ascii <= ASC_z) || (ascii >= ASC_0 && ascii <= ASC_9)) // if the key pressed is number or letter, print
    {
        printf("%c", ascii);
    }
}
