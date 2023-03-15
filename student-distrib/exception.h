#include "lib.h"

extern void init_idt();

// Exceptions


// extern uint32_t *exception_handler[NUM_EXCEPTION];
// extern void exception_handler_wrapper();

extern void division_error();
// extern void debug();
// extern void non_maskable_interrupt();
// extern void breakpoint();
// extern void overflow();
// extern void bound_range_exceeded();
// extern void invalid_opcode();
// extern void device_not_available();
// extern void double_fault();
// extern void coprocessor_segment_overrun();
// extern void invalid_tss();
// extern void segment_not_present();
// extern void stack_segment_fault();
// extern void general_protection_fault();
// extern void page_fault();
// extern void reserved();
// extern void x87_floating_point_exception();
// extern void alignment_check();
// extern void machine_check();
// extern void simd_floating_point_exception();
// extern void virtualization_exception();
// extern void control_protection_exception();
// extern void hypervisor_injection_exception();
// extern void vmm_communication_exception();
// extern void security_exception();
