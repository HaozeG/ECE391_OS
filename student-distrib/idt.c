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
    idt[0].reserved0 = 0;
    idt[0].present = 1;
    idt[0].seg_selector = KERNEL_CS;
    SET_IDT_ENTRY(idt[0], DIVISION_ERROR);

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
