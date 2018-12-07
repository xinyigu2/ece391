#ifndef RTC_H
#define RTC_H
#include "lib.h"
#include "i8259.h"
#define RTC_REG_A 0x8A
#define RTC_REG_B 0x8B
#define RTC_PORT1 0x70
#define RTC_PORT2 0x71
#define RTC_IRQ 8

extern void rtc_handler();
void rtc_init();
void do_rtc_handler();
int set_freq(int freq);
int32_t rtc_read (uint8_t *buf, int32_t nbytes);
int32_t rtc_write (uint8_t *buf, int32_t nbytes);
int32_t rtc_open ();
int32_t rtc_close ();

#endif
