#include "exception.h"


static char *exception_msg[32] = {
"Division Error",
"Debug",
"Non-maskable Interrupt",
"Breakpoint",
"Overflow",
"Bound Range Exceeded",
"Invalid Opcode",
"Device Not Available",
"Double Fault",
"Coprocessor Segment Overrun",
"Invalid TSS",
"Segment Not Present",
"Stack-Segment Fault",
"General Protection Fault",
"Page Fault",
"Reserved",
"x87 Floating-Point Exception",
"Alignment Check",
"Machine Check",
"SIMD Floating-Point Exception",
"Virtualization Exception",
"Control Protection Exception",
"Reserved",
"Hypervisor Injection Exception",
"VMM Communication Exception",
"Security Exception",
"Reserved"
};

// void exception_handler_wrapper() {




// }
// DO_CALL(ECE391_TEMP,0);
// TODO: Use asm wrapper to deal with values on stack
void division_error() {
    // clear();
    printf("INTO TEST\n");
    printf("%s\n", exception_msg[0]);
    do {
    }while (1);
}