#include "rtc.h"
#include "systemcall.h"
#include "scheduler.h"

#define HIGHEST_FREQ 1024
#define TERMINAL_CNT 3
#define BIT6_MASK 0x40
#define BYTE_HIBITS 0xF0

static int rtc_int_flag[TERMINAL_CNT] = {0, 0, 0};//flag for rtc interrupt occurs
extern int curr_scheduled_terminal;
/*
rtc_init()
inputs: none
outputs: none
enable the rtc irq line, set the default registers
*/
//cited from https://wiki.osdev.org/
void rtc_init(){
    char prev;
    enable_irq(RTC_IRQ);
    outb(RTC_REG_B, RTC_PORT1);		     // select register B, and disable NMI
    prev = inb(RTC_PORT2);	          // read the current value of register B
    outb(RTC_REG_B, RTC_PORT1);		     // set the index again (a read will reset the index to register D)
    outb(prev | BIT6_MASK, RTC_PORT2);	   // write the previous value ORed with 0x40. This turns on bit 6 of register B
}


/*
rtc_handler()
inputs: none
outputs: none
handle the rtc interrupt
*/
void do_rtc_handler(){
    cli();
    send_eoi(RTC_IRQ);
    outb(0x0C, RTC_PORT1); 	//select register C
	inb(RTC_PORT2); 		//throw away contents
    rtc_int_flag[curr_scheduled_terminal]++;       //set flag to 1
    sti();
}

/*
set_freq()
inputs: frequency in hz
outputs: none
change the rtc frequency
return 0 if success
*/
int set_freq(int freq){
    char rate,prev;
    //set rate according to frequency
    if(freq > 8192 || freq <2){     // max frequency is 8192 = 2^13
        return -1; // frequency too high or too low
    }
    // frequency must be power of 2
    if(freq == 2){
        rate = 0x0F;    //register value corresponding to freq 2^1 = 2
    }else if(freq == 4){
        rate = 0x0E;    //register value corresponding to freq 2^2 = 4
    }else if(freq == 8){
        rate = 0x0D;    //register value corresponding to freq 2^3 = 8
    }else if(freq == 16){
        rate = 0x0C;    //register value corresponding to freq 2^4 = 16
    }else if(freq == 32){
        rate = 0x0B;    //register value corresponding to freq 2^5 = 32
    }else if(freq == 64){
        rate = 0x0A;    //register value corresponding to freq 2^6 = 64
    }else if(freq == 128){
        rate = 0x09;    //register value corresponding to freq 2^7 = 128
    }else if(freq == 256){
        rate = 0x08;    //register value corresponding to freq 2^8 = 256
    }else if(freq == 512){
        rate = 0x07;    //register value corresponding to freq 2^9 = 512
    }else if(freq == 1024){
        rate = 0x06;    //register value corresponding to freq 2^10 = 1024
    }else if(freq == 2048){
        rate = 0x05;    //register value corresponding to freq 2^11 = 2048
    }else if(freq == 4096){
        rate = 0x04;    //register value corresponding to freq 2^12 = 4096
    }else if(freq == 8192){
        rate = 0x03;    //register value corresponding to freq 2^13 = 8192
    }else{
        return -1;//frequency not correct
    }                 // rate must be above 2 and not over 15
    cli();//not sure whether we need this
    outb(RTC_REG_A, RTC_PORT1);                   // set index to register A, disable NMI
    prev = inb(RTC_PORT2);                   // get initial value of register A
    outb(RTC_REG_A, RTC_PORT1);                   // reset index to A
    outb((prev & BYTE_HIBITS) | rate, RTC_PORT2);   //write only our rate to A. Note, rate is the bottom 4 bits.
    sti();
    return 0;
}

/*
rtc_open()
inputs: const uint8_t* filename
outputs: return 0 if success
initializes RTC frequency to maximum frequency, return 0
*/
int32_t rtc_open (){
    cli();
    if(set_freq(HIGHEST_FREQ) == -1){
        sti();
        return -1;//set frequency not success
    }
    sti();
    return 0;
}

/*
rtc_close()
inputs: none
outputs: return 0 if success
 probably does nothing, unless you virtualize RTC, return 0
 */
int32_t rtc_close (){
    return 0;
}

/*
rtc_read()
inputs: void* buf, int32_t nbytes
outputs: return 0 if success
should block until the next interrupt, return 0
*/
int32_t rtc_read (uint8_t *buf, int32_t nbytes){
    int fd, pid = get_terminal_running_pid(curr_scheduled_terminal);
    PCB_t* curr_pcb = get_pcb_by_pid(pid);
    fd = curr_pcb->current_fd;

    if(fd <= 1 || fd >= MAX_PROCESS)
        return -1;

    int32_t current_freq = (curr_pcb->opened_files[fd]).file_position;

    if(!current_freq)
        return -1;
    current_freq = HIGHEST_FREQ/current_freq;
    while(rtc_int_flag[curr_scheduled_terminal] < current_freq/TERMINAL_CNT){}
    cli();
    rtc_int_flag[curr_scheduled_terminal] = 0;
    sti();

    return 0;
}

/*
rtc_write()
inputs: const void* buf, int32_t nbytes
outputs: return 0 if success
change frequency， always receive 4 bytes
*/
int32_t rtc_write (uint8_t *buf, int32_t nbytes){
    if(nbytes != 4 || buf == NULL){
        return -1; //fail, only receives 4 bytes
    }

    int fd, pid = get_terminal_running_pid(curr_scheduled_terminal);
    PCB_t* curr_pcb = get_pcb_by_pid(pid);
    fd = curr_pcb->current_fd;

    if(fd <= 1 || fd >= MAX_PROCESS)
        return -1;
    /* stores the current rtc frequency to file position of rtc file */
    (curr_pcb->opened_files[fd]).file_position = (*(int32_t*)buf);

    return nbytes;
}
