// idt.h - IDT related

#ifndef _IDT_H
#define _IDT_H


#define NUM_EXCEPTION 32

extern void init_idt();


// Interrputs
#define KEYBOARD_VEC 0x21
#define RTC_VEC 0x28


// Syscall
#define SYS_CALL_VEC 0x80

#endif
