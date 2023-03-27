

#ifndef _RTC_H
#define _RTC_H


#include "types.h"
//initialize RTC
extern void rtc_init(void);
//handling RTC
extern void rtc_handler(void);
//reading RTC
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
//writing RTC
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
//opening RTC
int32_t rtc_open(const uint8_t* filename);
//closing RTC
int32_t rtc_close(int32_t fd);
//setting Rate RTC
void set_freq(int32_t target_freq);


#endif

