
#include "lib.h"
#include "i8259.h"
#include "rtc.h"


#define RTC_PORT 0x70 //RTC port
#define RTC_DATA 0x71 //RTC data port
#define RTC_REG_A 0x8A //register A RTC
#define RTC_REG_B 0x0B //register B RTC

volatile int flag_wait = 0; //used to communicate when to block interrupt
/*
* rtc_init
*   DESCRIPTION: Initialize RTC, turn on the IRQ 
*                  with the default 1024 Hz rate
*   INPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: none
*/
void rtc_init(void) {
    outb(RTC_REG_B, RTC_PORT);      //select register B and disable NMI
    char prev = inb(RTC_DATA);        //read current value at register B
    outb(RTC_REG_B, RTC_PORT);        //set the index again
    outb(prev | 0x40, RTC_DATA);       //write previous value ORed with 0x40. turns on six bit of register B
    outb(RTC_REG_A, RTC_PORT);            //disable NMI
    outb(0x06, RTC_DATA);           //frequency set to 1024
    enable_irq(8);             //RTC at irq 8
}

/*
* rtc_handler
*   DESCRIPTION: executes test interrupt handler when an RTC interrupt occurs. handles interrupts using register C
*   INPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: none
*/
void rtc_handler(void) {
    cli();
    outb(0x0C, RTC_PORT);  //selects register C
    inb(RTC_DATA);    //throw away contents
    if (get_rtc_switch() == 1) {
        test_interrupts();    //call test_interrupts
    }
    flag_wait = 1;    //tells read to block interrupt
    sti();
    send_eoi(8); //sends eoi after servicing interrupt
}

