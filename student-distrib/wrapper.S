#define ASM     1
; #include "wrapper.h"

#define EXCEPT_LINK(name, func, num)   \
    .GLOBL name                     ;\
    name:                           ;\
        pushal                      ;\
        pushfl                      ;\
        pushl $num                  ;\
        call func                   ;\
        addl $4, %esp               ;\
        popfl                     ;\
        popal                     ;\
        iret                      ;\

EXCEPT_LINK(DIVISION_ERROR, division_error, 0)


#define INTR_LINK(name,func)    \
    .GLOBL name                 ;\
    name:                       ;\
        pushal                  ;\
        pushfl                  ;\
        call func               ;\
        popfl                   ;\
        popal                   ;\
        iret                    ;\




#define DO_CALL(name_call,number)       \
    .GLOBL name_call                    ;\
    name_call:   PUSHL	%EBX            ;\
	    MOVL	$number,%EAX            ;\
	    MOVL	8(%ESP),%EBX            ;\
	    MOVL	12(%ESP),%ECX           ;\
	    MOVL	16(%ESP),%EDX           ;\
	    INT	$0x80                       ;\
	    POPL	%EBX                    ;\
	    RET

; DO_CALL(ece391_temp,0)
