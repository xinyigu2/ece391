#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "i8259.h"
#define PIT_IRQ 0
#define TERMINAL_CNT 3
#define MAX_PROCESS 6

extern int curr_terminal_pid[TERMINAL_CNT][MAX_PROCESS];
extern int launch_flags[TERMINAL_CNT];

extern void pit_handler();
void pit_init();
void do_pit_handler();
void schedule();
int32_t get_terminal_running_pid(int32_t terminal);
int get_curr_scheduled_terminal();
uint32_t rand();

#endif
