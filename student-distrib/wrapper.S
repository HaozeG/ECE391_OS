#define ASM 1

/* 
    Wrapper for exception handlers
    input:  name - function name
            func - handler function
            num - interrupt vector
    Without error code: use 0 as error code
*/    
#define EXCEPT_LINK(name, func, num)   \
    .GLOBL name                       ;\
    name:                             ;\
        pushal                        ;\
        pushfl                        ;\
        pushl $0                      ;\
        pushl $num                    ;\
        call func                     ;\
        addl $8, %esp                 ;\
        popfl                        ;\
        popal                        ;\
        iret                        ;\

/*  
    With error code already on stack
    Ref: Linux 0.11 source code 
*/
#define EXCEPT_LINK_ERRORCODE(name, func, num)   \
    .GLOBL name                     ;\
    name:                           ;\
        xchgl %eax,4(%esp)          ;\
        pushal                      ;\
        pushfl                      ;\
        pushl %eax                  ;\
        pushl $num                  ;\
        call func                   ;\
        addl $8, %esp               ;\
        popfl                     ;\
        popal                     ;\
        iret                      ;\

/* 
    Wrapper for interrupts(not handler)
    input:  name - function name
            num - index to system call
    TODO: how to deal with pass in num
*/
#define INTR_LINK(name,func)    \
    .GLOBL name                 ;\
    name:                       ;\
        pushal                  ;\
        pushfl                  ;\
        cli                     ;\
        call func               ;\
        popfl                   ;\
        popal                   ;\
        iret                    ;\


/* 
    Wrapper for system calls function(not handler)
    input:  name - function name
            num - index to system call
    TODO: how to deal with pass in num
*/

; #define DO_CALL(name, num)       \
;     .GLOBL name                        ;\
;     name:   
;         PUSHL	%EBX                    ;\
; 	    MOVL	$0, %EAX                ;\
; 	    MOVL	8(%ESP),%EBX            ;\
; 	    MOVL	12(%ESP),%ECX           ;\
; 	    MOVL	16(%ESP),%EDX           ;\
; 	    INT	$0x80                       ;\
; 	    POPL	%EBX                    ;\
; 	    RET                             ;\

/*
    Numbers are related to vector number defined by Intel
    Ref: https://wiki.osdev.org/Exceptions
*/
EXCEPT_LINK(DIVISION_ERROR, common_exception_handler, 0)
EXCEPT_LINK(DEBUG, common_exception_handler, 1)
EXCEPT_LINK(NON_MASKABLE_INTERRUPT, common_exception_handler, 2)
EXCEPT_LINK(BREAKPOINT, common_exception_handler, 3)
EXCEPT_LINK(OVERFLOW, common_exception_handler, 4)
EXCEPT_LINK(BOUND_RANGE_EXCEEDED, common_exception_handler, 5)
EXCEPT_LINK(INVALID_OPCODE, common_exception_handler, 6)
EXCEPT_LINK(DEVICE_NOT_AVAILABLE, common_exception_handler, 7)
EXCEPT_LINK_ERRORCODE(DOUBLE_FAULT, common_exception_handler, 8)
EXCEPT_LINK_ERRORCODE(INVALID_TSS, common_exception_handler, 10)
EXCEPT_LINK_ERRORCODE(SEGMENT_NOT_PRESENT, common_exception_handler, 11)
EXCEPT_LINK_ERRORCODE(STACK_SEGMENT_FAULT, common_exception_handler, 12)
EXCEPT_LINK_ERRORCODE(GENERAL_PROTECTION_FAULT, common_exception_handler, 13)
EXCEPT_LINK_ERRORCODE(PAGE_FAULT, common_exception_handler, 14)
EXCEPT_LINK(X87_FLOATING_POINT_EXCEPTION, common_exception_handler, 16)
EXCEPT_LINK_ERRORCODE(ALIGNMENT_CHECK, common_exception_handler, 17)
EXCEPT_LINK(MACHINE_CHECK, common_exception_handler, 18)
EXCEPT_LINK(SIMD_FLOATING_POINT_EXCEPTION, common_exception_handler, 19)
EXCEPT_LINK(VIRTUALIZATION_EXCEPTION, common_exception_handler, 20)
EXCEPT_LINK_ERRORCODE(CONTROL_PROTECTION_EXCEPTION, common_exception_handler, 21)
EXCEPT_LINK(HYPERVISOR_INJECTION_EXCEPTION, common_exception_handler, 28)
EXCEPT_LINK_ERRORCODE(VMM_COMMUNICATION_EXCEPTION, common_exception_handler, 29)
EXCEPT_LINK_ERRORCODE(SECURITY_EXCEPTION, common_exception_handler, 30)
EXCEPT_LINK(RESERVED, common_exception_handler, 31)

INTR_LINK(INTR_ECE391_TEMP, ece391_temp)

INTR_LINK(KEYBOARD_INTERRUPT,keyboard_handler)

/*
    System call function for test
    0x80: interrupt vector for system call 
*/
.GLOBL ECE391_TEMP
ECE391_TEMP:
    INT $0x80
