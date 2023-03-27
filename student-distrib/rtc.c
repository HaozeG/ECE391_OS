#include "lib.h"
#include "i8259.h"
#include "rtc.h"
#include "idt.h"


#define RTC_PORT 0x70 //RTC port
#define RTC_DATA 0x71 //RTC data port
#define RTC_REG_A 0x8A //register A RTC
#define RTC_REG_B 0x8B //register B RTC
#define RTC_REG_C 0x8C //register C            


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
   
    // // set rate
    // outb(RTC_REG_A, RTC_PORT);            //set index to reg A
    // prev = inb(RTC_DATA);
    // outb(RTC_REG_A, RTC_PORT);            //reset index to A
    // // outb((prev & 0xF0) | 0x06, RTC_DATA);           //frequency set to 1024
    // outb(0x06, RTC_DATA);           //frequency set to 1024
    set_freq(2);          //frequency set to 2 which is min
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
    send_eoi(RTC_VEC - IRQ_BASE_VEC); //sends eoi after servicing interrupt
    flag_wait = 1;    //tells read to block interrupt
}


/*
* rtc_open
*   DESCRIPTION: used to open RTC
*   INPUTS: filename is the string filename
*   RETURN VALUE: 0
*   SIDE EFFECTS: none
*/
int32_t rtc_open(const uint8_t* filename)
{
     set_freq(2);  //setting frequency to 2Hz
     return 0;
}


/*
* rtc_close
*   DESCRIPTION: used to close RTC
*   INPUTS: fd is the fule descriptor number
*   RETURN VALUE: 0
*   SIDE EFFECTS: none
*/
int32_t rtc_close(int32_t fd)
{
    return 0;
}


/*
* rtc_read
*   DESCRIPTION: used to read RTC
*   INPUTS: fd is the fule descriptor number, buf is the output data pointer, and nbytes is the number of bytes read
*   RETURN VALUE: 0 is success and -1 is failure
*   SIDE EFFECTS: none
*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {    
    flag_wait = 0;
    while(flag_wait == 0);     //used to wait until interrupt
    return 0;
}


/*
* rtc_write
*   DESCRIPTION: used to write RTC interrupt rate
*   INPUTS: fd is the fule descriptor number, buf is the output data pointer, and nbytes is the number of bytes
*   RETURN VALUE: 0 on success, -1 failure
*   SIDE EFFECTS: none
*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {  
    int32_t freq;
   
    if((nbytes != 4) || ((int32_t)buf == NULL))   //checks to make sure within the size
        return -1;
    else
        freq = *((int32_t*)buf);


    cli();
    set_freq(freq);       //setting the frequency
    sti();


    return nbytes;
}


/*
* set_freq
*   DESCRIPTION: setting the frequency rate
*   INPUTS: freq_final
*   RETURN VALUE:none
*   SIDE EFFECTS: none
*/
void set_freq(int32_t freq_final)
{
    char freq;
    unsigned char prev;
   
    outb(RTC_REG_A, RTC_PORT);    //Register A
    prev = inb(RTC_DATA);
   
    switch(freq_final){
        case 8192:
        case 4096:
        case 2048:                        //used RTC datasheet for all of these values until line 162
            return;
        case 1024:
            freq = 0x06;
            break;
        case 512:
            freq = 0x07;
            break;
        case 256:
            freq = 0x08;
            break;
        case 128:
            freq = 0x09;
            break;
        case 64:  
            freq = 0x0A;
            break;
        case 32:  
            freq = 0x0B;
            break;
        case 16:  
            freq = 0x0C;
            break;
        case 8:  
            freq = 0x0D;
            break;
        case 4:  
            freq = 0x0E;
            break;
        case 2:  
            freq = 0x0F;
            break;
       
        default: return;
    }
    outb(RTC_REG_A, RTC_PORT);          //setting RS values
    outb( (prev & 0xF0) | freq, RTC_DATA);  //masking value
}
