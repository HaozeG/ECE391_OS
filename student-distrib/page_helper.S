#define ASM 1
#include "paging.h"

.text
.globl loadPageDirectory, enablePaging
loadPageDirectory:
    push %ebp
    mov %esp, %ebp
    mov 8(%esp), %eax
    mov %eax, %cr3
    mov %ebp, %esp
    pop %ebp
    ret

enablePaging:
    push %ebp
    mov %esp, %ebp
    mov %cr4, %eax
    or $0x00000010, %eax # this is enabling 4 MiB pages
    mov %eax, %cr4
    mov %cr0, %eax
    or $0x80000001, %eax # enable paging and protected mode. 
    mov %eax, %cr0
    mov %ebp, %esp
    pop %ebp
    ret

