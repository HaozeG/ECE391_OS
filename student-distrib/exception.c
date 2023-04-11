#include "exception.h"
#include "types.h"
#include "syscall.h"

// Array of exception messages
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
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

/*
* common_exception_handler
*   DESCRIPTION: Handle exceptions that shows a "blue screen"
*           Exception message would be shown. Additional message
*           would follow based on error code if it is valid.
*   INPUTS: num - vector number of invoked exception
*           error code - 0 if no error code is generated or generated
*                       error code is 0
*                       or 32-bit value configured differently for each
*                       exception
*   RETURN VALUE: none
*   SIDE EFFECTS: Kernel would stuck in infinite loop; Only error code of 
*               page fault is configured
*/
void common_exception_handler(int32_t num, int32_t error_code) {
    // clear();
    set_color(0x0C, 1);
    printf("%s\n", exception_msg[num]);
    if (error_code != 0) {
        printf("Error code: %x\n", error_code);
        // Error code
        if (num == 14) {
            uint32_t error_address;
            // Page fault
            //  31              15                             4               0
            // +---+--  --+---+-----+---+--  --+---+----+----+---+---+---+---+---+
            // |   Reserved   | SGX |   Reserved   | SS | PK | I | R | U | W | P |
            // +---+--  --+---+-----+---+--  --+---+----+----+---+---+---+---+---+
            if (error_code & 0x0001) {
                printf("- Page-Protection Violation!\n");
            } else {
                printf("- Non-Present Page!\n");
            }
            if ((error_code >> 1) & 0x0001) {
                printf("- caused by WRITE ACCESS\n");
            } else {
                printf("- caused by READ ACCESS\n");
            }
            if ((error_code >> 2) & 0x0001) {
                printf("- privilege violation: CPL = 3\n");
            }
            if ((error_code >> 4) & 0x0001) {
                printf("- caused by INSTRUCTION FETCH\n");
            }
            asm volatile ("movl %%cr2, %0"   \
                :  "=r"(error_address)        \
                :                           \
                : "memory"                  \
            );
            printf("- Error virtual address: 0x%x\n", error_address);
        }
    }
    set_color(0x07, 1);
    // return 256 to indicate exception
    // TODO: deal with return value
    sys_halt((uint16_t)256);

    // do {
    // }while (1);
}
