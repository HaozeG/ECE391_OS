#include "idt.h"
#include "lib.h"
#include "x86_desc.h"
#include "wrapper.h"
// #include "wrapper.h"

// uint32_t *exception_handler[NUM_EXCEPTION] = {
//     &division_error,
//     &debug,
//     &non_maskable_interrupt,
//     &breakpoint,
//     &overflow,
//     &bound_range_exceeded
//     // &invalid_opcode(),
//     // &device_not_available(),
//     // &double_fault(),
//     // &coprocessor_segment_overrun(),
//     // &invalid_tss(),
//     // &segment_not_present(),
//     // &stack_segment_fault(),
//     // &general_protection_fault(),
//     // &page_fault(),
//     // &reserved(),
//     // &x87_floating_point_exception(),
//     // &alignment_check(),
//     // &machine_check(),
//     // &simd_floating_point_exception(),
//     // &virtualization_exception(),
//     // &control_protection_exception(),
//     // &reserved(),
//     // &hypervisor_injection_exception(),
//     // &vmm_communication_exception(),
//     // &security_exception(),
//     // &reserved(),
// }


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

void init_idt() {
    int i;
    // for (i = 0; i < 32; i++) {
    //     // Fill in IDT descriptors
    //     idt[i].dpl = 0;
    //     idt[i].present = 1;
    //     SET_IDT_ENTRY(idt[i], exception_handler());

    // }

    // Exceptions
    // 32-bit Trap gate
    // TODO: Exception 
    idt[0].size = 1;
    idt[0].reserved1 = 1;
    idt[0].reserved2 = 1;
    idt[0].reserved3 = 1;
    // DPL = 0(kernel)
    idt[0].dpl = 0;
    idt[0].present = 1;
    SET_IDT_ENTRY(idt[0], &DIVISION_ERROR);

    // Interrupts

    // System call
    // SET_IDT_ENTRY(idt[0x80], ECE391_TEMP);
    
    lidt(idt_desc_ptr); // Load IDTR

    printf("Enabling Interrupts\n");
    // sti();
};

// Exception
// void exception_handler() {
//     int32_t vector;
//     int32_t error_ip = 0;
//     asm volatile ("                 \
//         movl %eip, %0              \
//         "                           \
//         : "=r" (error_ip)                  \
//         :                             \
//         : "memory"                       \   
//     );                                  \
//     if (!error_ip) {
//         printf("%s: Invalid instruction \"%x\"\n", exception_msg[vector], error_ip);
//     }
//     printf("%s\n", exception_msg[vector]);

// }

// Interrupt handlers

// Syscall
// void ece391_temp () {
//     clear();
//     printf("SYSCALL! STUCK IN LOOP...\n");
//     do {
//     }while (1);
// };
