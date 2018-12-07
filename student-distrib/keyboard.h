#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "lib.h"

#define KEYBOARD_PORT1 0x60
#define CAPSLOCK 0x3A
#define LEFTSHIFT 0x2A
#define RIGHTSHIFT 0x36
#define LEFTALT 0x38
#define LEFTALTRELEASE 0xB8
#define BACKSPACE 0x0E
#define LEFTCONTROL 0x1D
#define LCONTROLRELEASE 0x9D
#define SPACE 0x39
#define LSHIFTRELEASE 0xAA
#define RSHIFTRELEASE 0xB6
#define LPRESS 0x26
#define FONE 0x3B
#define FTWO 0x3C
#define FTHREE 0x3D
#define ENTER 0x1C
#define KEYBOARD_IRQ 1
#define VIDEO_SIZE 4096
#define BUFFER_SIZE 128
#define TERMINAL_CNT 3
#define LARGEST_SCAN_CODE 0x3B
#define SCAN_CODE_BUF_SIZE 100

extern int curr_term;

extern void keyboard_handler();
void keyboard_init();
void do_keyboard_handler();
void terminal_init();
int terminal_open(const uint8_t* filename);
int terminal_close();
int terminal_read(unsigned char* buf, int size);
int terminal_write(unsigned char* buf, int size);
int terminal_switch(int term);
void map_video_page(int i);
unsigned int get_display_terminal();
unsigned char getScancode();

#endif
