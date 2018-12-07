#ifndef IDT_H
#define IDT_H
#include "sys_call_handler.h"

#define EXC_NUM 32
#define TAB_NUM 256
#define KEYBOARD_ENTRY 0x21
#define RTC_ENTRY 0x28
#define MOUSE_ENTRY 0x2C
#define PIT_ENTRY 0x20
#define SYS_CALL_ENTRY 0x80

void default_interrupt_handler();
void init_idt();

#endif
