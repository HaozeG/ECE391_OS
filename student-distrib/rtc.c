#include "lib.h"
#include "i8259.h"
#include "rtc.h"
#include "idt.h"

static int rtc_switch = 0;

#define RTC_PORT 0x70 //RTC port
#define RTC_DATA 0x71 //RTC data port
#define RTC_REG_A 0x8A //register A RTC
#define RTC_REG_B 0x8B //register B RTC
#define RTC_REG_C 0x8C //register C RTC

#define RTC_IRQ 8

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
    cli();
    // Turn on with 1024Hz
    outb(RTC_REG_B, RTC_PORT);      //select register B and disable NMI
    char prev = inb(RTC_DATA);        //read current value at register B
    outb(RTC_REG_B, RTC_PORT);        //set the index again
    outb(prev | 0x40, RTC_DATA);       //write previous value ORed with 0x40. turns on six bit of register B
    
    // set rate
    outb(RTC_REG_A, RTC_PORT);            //set index to reg A
    prev = inb(RTC_DATA);
    outb(RTC_REG_A, RTC_PORT);            //reset index to A
    // outb((prev & 0xF0) | 0x06, RTC_DATA);           //frequency set to 1024
    outb(0x06, RTC_DATA);           //frequency set to 1024

    enable_irq(RTC_VEC - IRQ_BASE_VEC);
}

/*
* rtc_handler
*   DESCRIPTION: executes test interrupt handler when an RTC interrupt occurs. handles interrupts using register C
*   INPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: none
*/
void rtc_handler(void) {
    outb(RTC_REG_C, RTC_PORT);  //selects register C
    inb(RTC_DATA);    //throw away contents
    send_eoi(RTC_IRQ); //sends eoi after servicing interrupt

    if (get_rtc_switch() == 1) {
        test_interrupts();    //call test_interrupts
    }
    flag_wait = 1;    //tells read to block interrupt
}

/* void rtc_on(void)
 * Inputs: void
 * Return Value: void
 * Function: turns on rtc switch for rtc interrupt
*/
void rtc_on(void)
{
    rtc_switch = 1;
    return;
}

/* void rtc_off
 * Inputs: void
 * Return Value: void
 * Function: turns off rtc switch for rtc interrupt
 */
void rtc_off(void)
{
    rtc_switch = 0;
    return;    
}

/* int get_rtc_switch(void)
 * Inputs: void
 * Return Value: rtc_switch
 * Function: return rtc_switch
 */
int get_rtc_switch(void)
{
    return rtc_switch;
}
