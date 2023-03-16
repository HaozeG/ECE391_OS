#include "idt.h"
#include "lib.h"
#include "x86_desc.h"
#include "wrapper.h"


// Array of function addresses for SET_IDT_ENTRY
void *exception_handlers[NUM_EXCEPTION] = {
    DIVISION_ERROR,
    DEBUG,
    NON_MASKABLE_INTERRUPT,
    BREAKPOINT,
    OVERFLOW,
    BOUND_RANGE_EXCEEDED,
    INVALID_OPCODE,
    DEVICE_NOT_AVAILABLE,
    DOUBLE_FAULT,
    RESERVED,
    INVALID_TSS,
    SEGMENT_NOT_PRESENT,
    STACK_SEGMENT_FAULT,
    GENERAL_PROTECTION_FAULT,
    PAGE_FAULT,
    RESERVED,
    X87_FLOATING_POINT_EXCEPTION,
    ALIGNMENT_CHECK,
    MACHINE_CHECK,
    SIMD_FLOATING_POINT_EXCEPTION,
    VIRTUALIZATION_EXCEPTION,
    CONTROL_PROTECTION_EXCEPTION,
    RESERVED,
    HYPERVISOR_INJECTION_EXCEPTION,
    VMM_COMMUNICATION_EXCEPTION,
    SECURITY_EXCEPTION,
    RESERVED
};

/*
* init_idt
*   DESCRIPTION: Initialize IDT, including filling in IDT descriptors
*               and load IDTR register. New interrupts or system calls
*               can be modified here.
*   INPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: All entries reserved by Intel are set with a handler 
*           that outputs "Reserved" message;
*/
void init_idt() {
    int i;
    // Exceptions
    for (i = 0; i < NUM_EXCEPTION; i++) {
        // 32-bit
        idt[i].size = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        // Trap gate
        idt[i].reserved3 = 1;
        // DPL = 0(kernel)
        idt[i].dpl = 0;
        idt[i].reserved0 = 0;
        idt[i].present = 1;
        idt[i].seg_selector = KERNEL_CS;
        SET_IDT_ENTRY(idt[i], exception_handlers[i]);
    }

    for (i = NUM_EXCEPTION; i < NUM_VEC; i++) {
        // 32-bit
        idt[i].size = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        // Interrupt gate
        idt[i].reserved3 = 0;
        // DPL = 0(kernel)
        idt[i].dpl = 0;
        idt[i].reserved0 = 0;
        // disable all
        idt[i].present = 0;
        idt[i].seg_selector = KERNEL_CS;
        SET_IDT_ENTRY(idt[i], exception_handlers[NUM_EXCEPTION - 1]);    // Reserved handler
    }

    // System call
    // DPL = 3(user)
    idt[SYS_CALL_VEC].dpl = 3;
    // enable
    idt[SYS_CALL_VEC].present = 1;
    SET_IDT_ENTRY(idt[SYS_CALL_VEC], INTR_ECE391_TEMP);

    // Interrupts
    // Keyboard
    idt[KEYBOARD_VEC].present = 1;
    SET_IDT_ENTRY(idt[KEYBOARD_VEC], KEYBOARD_INTERRUPT);
    // RTC
    // idt[RTC_VEC].present = 1;
    // SET_IDT_ENTRY(idt[RTC_VEC], rtc_handler);

    lidt(idt_desc_ptr); // Load IDTR

    // printf("Enabling Interrupts\n");
    // TODO: Uncomment here after interrupts are set
    sti();
};

// Syscall that print a message and stuck in loop
void ece391_temp () {
    printf("SYSCALL! STUCK IN LOOP...\n");
    do {
    }while (1);
};
