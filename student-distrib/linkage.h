#if !defined(LINKAGE_H)
#define LINKAGE_H

#include "types.h"
#include "keyboard.h"
#include "rtc.h"
#include "syscall.h"

#ifndef ASM



// Exception handlers
extern void DIVISION_ERROR();
extern void DEBUG();
extern void NON_MASKABLE_INTERRUPT();
extern void BREAKPOINT();
extern void OVERFLOW();
extern void BOUND_RANGE_EXCEEDED();
extern void INVALID_OPCODE();
extern void DEVICE_NOT_AVAILABLE();
extern void DOUBLE_FAULT();
extern void COPROCESSOR_SEGMENT_OVERRUN();
extern void INVALID_TSS();
extern void SEGMENT_NOT_PRESENT();
extern void STACK_SEGMENT_FAULT();
extern void GENERAL_PROTECTION_FAULT();
extern void PAGE_FAULT();
extern void RESERVED();
extern void X87_FLOATING_POINT_EXCEPTION();
extern void ALIGNMENT_CHECK();
extern void MACHINE_CHECK();
extern void SIMD_FLOATING_POINT_EXCEPTION();
extern void VIRTUALIZATION_EXCEPTION();
extern void CONTROL_PROTECTION_EXCEPTION();
extern void RESERVED();
extern void HYPERVISOR_INJECTION_EXCEPTION();
extern void VMM_COMMUNICATION_EXCEPTION();
extern void SECURITY_EXCEPTION();
extern void RESERVED();

// Wrapped function to invoke system call
extern void ECE391_TEMP();
// Wrapped system call handler
extern int32_t dispatch_syscall(int32_t num);
// Specific system call handler
extern void ece391_temp();

// wrapped keyboard handler
extern void KEYBOARD_INTERRUPT();
extern void RTC_INTERRUPT();

int32_t dummy;  // not used


#endif

#endif
