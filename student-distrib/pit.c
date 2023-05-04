#include "pit.h"
#include "lib.h"
#include "i8259.h"
#include "idt.h"
#include "scheduler.h"
#include "vga.h"

#define PIT_DIVIDER 11931   // set T=10ms, floor(1193182Hz*10ms)
// #define PIT_DIVIDER 35793   // set T=30ms, floor(1193182Hz*30ms)
#define PIT_MODE 0x36   // channel 0, lobite/hibite, square wave generator
#define PIT_DATA_MASK 0x0F  // used for 8-bit data port
#define PIT_DATA_PORT_0 0x40
#define PIT_COMMAND_REG 0x43
/* Usage of command register
Bits         Usage
6 and 7      Select channel :
                0 0 = Channel 0
                0 1 = Channel 1
                1 0 = Channel 2
                1 1 = Read-back command (8254 only)
4 and 5      Access mode :
                0 0 = Latch count value command
                0 1 = Access mode: lobyte only
                1 0 = Access mode: hibyte only
                1 1 = Access mode: lobyte/hibyte
1 to 3       Operating mode :
                0 0 0 = Mode 0 (interrupt on terminal count)
                0 0 1 = Mode 1 (hardware re-triggerable one-shot)
                0 1 0 = Mode 2 (rate generator)
                0 1 1 = Mode 3 (square wave generator)
                1 0 0 = Mode 4 (software triggered strobe)
                1 0 1 = Mode 5 (hardware triggered strobe)
                1 1 0 = Mode 2 (rate generator, same as 010b)
                1 1 1 = Mode 3 (square wave generator, same as 011b)
0            BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD
*/
// counter to divide frequency from 100Hz
static int16_t cnt = 1;

/*
* pit_init
*   DESCRIPTION: Initialize PIT to square wave generator mode, frequency set to ~100Hz, enable IRQ0
*   INPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: enable IRQ 0
*/
void pit_init() {
    cli();
    // set mode
    outb(PIT_MODE, PIT_COMMAND_REG);
    // set divider(frequency)
    outb((uint8_t)(PIT_DIVIDER & PIT_DATA_MASK), PIT_DATA_PORT_0);
    outb((uint8_t)(PIT_DIVIDER >> 8) & PIT_DATA_MASK, PIT_DATA_PORT_0);
    enable_irq(PIT_VEC - IRQ_BASE_VEC);
}

/*
* pit_handler
*   DESCRIPTION: Handler for PIT interrupt, trigger scheduler
*   INPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: trigger scheduler to switch task
*/
void pit_handler() {
    // cli();
    send_eoi(PIT_VEC - IRQ_BASE_VEC);
    if (!--cnt) {
        cnt = 4;  // 25Hz signal
        if (is_mode_X) {
            show_screen();
        }
        // putc('t');
        // schedule();
    }
    // sti();
    // trigger scheduler
    if (!is_mode_X){
        schedule();
    }
}
