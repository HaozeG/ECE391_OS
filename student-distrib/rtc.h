
#ifndef _RTC_H
#define _RTC_H

#include "types.h"

extern void rtc_init(void);
extern void rtc_handler(void);

void rtc_on(void);
void rtc_off(void);
int get_rtc_switch(void);

#endif
