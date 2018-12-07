#include "scheduler.h"
#include "file_sys.h"
#include "paging.h"
#include "systemcall.h"
#include "x86_desc.h"
#include "keyboard.h"
#include "paging.h"

#define CONST1 19
#define CONST2 8
#define CONST3 23

#define CHANNEL0_DATA 0x40
#define COMMAND_PORT 0x43
#define LOBYTE_MASK 0xFF
#define HIBYTE_MASK 0xFF00
#define BYTE_SIZE 8
#define MAX_RESET_VAL 65535
#define INT_COMMAND 0x30 // 00 (channel 0) 11 (lobyte/hibyte) 000 (interrupt on count) 0
#define EMPTY -1

static int boot_time = 0;
int launch_flags[TERMINAL_CNT] = {0,0,0};
int curr_terminal_pid[TERMINAL_CNT][MAX_PROCESS] = {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
                                                   {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
                                                   {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}};
int curr_scheduled_terminal = 0;

/*
get_curr_scheduled_terminal()
inputs: none
outputs: none
return value: currently scheduler terminal index
description: Simple helper function to get the currently
             scheduled terminal
*/
int get_curr_scheduled_terminal(){
    return curr_scheduled_terminal;
}

/*
schedule()
inputs: none
outputs: none
description: This function is called by the PIT interrupt handler
to do the info-saving and context-switch to switch to the
program running on the next terminal
*/
void schedule(){
    int curr = 0;
    int switch_to = MAX_PROCESS - 1;
    PCB_t *target_pcb, *curr_pcb;
    if(launch_flags[0] || launch_flags[1] || launch_flags[2]){
        /* get pcb of current scheduled process */
        curr = get_terminal_running_pid(curr_scheduled_terminal);
        curr_pcb = get_pcb_by_pid(curr);
        curr_pcb->scheduled_esp0 = tss.esp0;

        asm volatile(
            "movl %%ebp, %0;"
            "movl %%esp, %1;"
             :"=&r"((curr_pcb->scheduled_ebp)), "=&r"((curr_pcb->scheduled_esp))
        );
        //printf("active: %d, on: %d, pid: %d, ebp: %x, esp: %x\n", curr_scheduled_terminal, curr_term, curr, curr_pcb->scheduled_ebp, curr_pcb->scheduled_esp);
        curr_scheduled_terminal = (1+curr_scheduled_terminal)%3;
    }

    if(launch_flags[0] && launch_flags[1] && launch_flags[2]){

        /* get pcb of process to switch to */
        switch_to = get_terminal_running_pid(curr_scheduled_terminal);
        target_pcb = get_pcb_by_pid(switch_to);
        tss.esp0 = target_pcb->scheduled_esp0;
        /* change page to the program to switch to */
        task_paging_setup((unsigned char *)PROGRAM_ADDR, switch_to);
        TLB_flushing();

        asm volatile(
            "movl %0, %%ebp;"
            "movl %1, %%esp;"
            :
            :"r"((target_pcb->scheduled_ebp)), "r"((target_pcb->scheduled_esp))
            :"esp", "ebp"
        );

    }
    else{
        /* launch 3 shells here */
        if(launch_flags[curr_scheduled_terminal]==0){

            terminal_switch(curr_scheduled_terminal);
            clear();
            launch_flags[curr_scheduled_terminal]=1;
            //curr_scheduled_terminal = (1+curr_scheduled_terminal)%3;
            printf("Launching terminal: %d\n", curr_scheduled_terminal);
            ece391_execute((uint8_t *)"shell");
        }
    }
}

/*
pit_init()
inputs: none
outputs: none
description: This function enable the timer chip interrup and
             set the PIT to interrupt mode with the number of
             count indicated by MAX_RESET_VAL
*/
void pit_init(){
    enable_irq(PIT_IRQ);
    /* set channel 0 reload value to 65535, the maximum reload value, which
     * makes the interval between interrupts roughly 50ms. Write the reset
     * value to the corresponding port, and set the PIT to interrupt mode
     */
    outb(MAX_RESET_VAL & LOBYTE_MASK, CHANNEL0_DATA);
    outb((MAX_RESET_VAL & HIBYTE_MASK) >> BYTE_SIZE, CHANNEL0_DATA);
    outb(INT_COMMAND, COMMAND_PORT);
}

/*
get_terminal_running_pid()
inputs: terminal: terminal to get process id on
outputs: none
description: This function returns the process id of the
             input terminal
*/
int32_t get_terminal_running_pid(int32_t terminal){
    int i;
    /* check for process running on top of every terminal */
    for(i = MAX_PROCESS - 1; i >= 0; i--){
        if(curr_terminal_pid[terminal][i] != EMPTY){
            return curr_terminal_pid[terminal][i];
        }
    }
    return -1;
}

/*
do_pit_handler()
inputs: none
outputs: none
description: This function sends eoi to the PIT_IRQ port, reload
             the number of count before interrupt and schedule the
             next process
*/
void do_pit_handler(){
    /* send eoi to PIC */
    send_eoi(PIT_IRQ);
    /* update boot time */
    boot_time +=1;
    /* rewrite reset value to PIT */
    outb(MAX_RESET_VAL & LOBYTE_MASK, CHANNEL0_DATA);
    outb((MAX_RESET_VAL & HIBYTE_MASK) >> BYTE_SIZE, CHANNEL0_DATA);
    schedule();
}

/*
rand()
inputs: none
outputs: A fake random number
description: Function that uses OS boot time to generate random number
*/
uint32_t rand(){
  return (CONST1*boot_time+CONST2)%CONST3;
}
