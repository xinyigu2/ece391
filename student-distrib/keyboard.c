#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "systemcall.h"
#include "paging.h"
#include "scheduler.h"



static unsigned char input_buffer[TERMINAL_CNT][BUFFER_SIZE];
static unsigned char* video_memory_terminal[TERMINAL_CNT];
static unsigned char screenx[TERMINAL_CNT];
static unsigned char screeny[TERMINAL_CNT];
static int buffer_idx[TERMINAL_CNT] = {0,0,0};  //from 0 to 127, more than 127, does not take anymore

static int newline_mark[TERMINAL_CNT] = {0, 0, 0};

static int CAPS_STATUS = 0;  //initialize as not pressed
static int SHIFT_STATUS = 0;
static int ALT_STATUS = 0;
static int CTRL_STATUS = 0;
int curr_term = 0;
extern int curr_scheduled_terminal;

//four tables of scancodes
//corresponding to the caps and shifts status
//100 is just a size
unsigned char scantable[SCAN_CODE_BUF_SIZE] = {'\0','`','1','2','3','4','5','6','7','8','9','0','-','=','\0',
                                ' ','q','w','e','r','t','y','u','i','o','p','[',']','\n',
                                '\0','a','s','d','f','g','h','j','k','l',';','\'','`','\0',
                                '\\','z','x','c','v','b','n','m',',','.','/','\0','\0','\0',' '};
                                                //this array is the scancode mapping


unsigned char captable[SCAN_CODE_BUF_SIZE] = {'\0','`','1','2','3','4','5','6','7','8','9','0','-','=','\0',
                                ' ','Q','W','E','R','T','Y','U','I','O','P','[',']','\n',
                                '\0','A','S','D','F','G','H','J','K','L',';','\'','`','\0',
                                '\\','Z','X','C','V','B','N','M',',','.','/','\0','\0','\0',' '};

unsigned char shifttable[SCAN_CODE_BUF_SIZE] = {'\0','~','!','@','#','$','%','^','&','*','(',')','_','+','\0',
                               ' ','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
                               '\0','A','S','D','F','G','H','J','K','L',':','\"' ,'~','\0',
                               '|','Z','X','C','V','B','N','M','<','>','?','\0','\0','\0',' '};


unsigned char shiftcap[SCAN_CODE_BUF_SIZE] = {'\0','~','!','@','#','$','%','^','&','*','(',')','_','+','\0',
                                ' ','q','w','e','r','t','y','u','i','o','p','{','}','\n',
                                '\0','a','s','d','f','g','h','j','k','l',':','\"' ,'~','\0',
                                '|','z','x','c','v','b','n','m','<','>','?','\0','\0','\0',' '};

/*
keyboard_init()
inputs: none
outputs: none
enable the keyboard irq line
*/
void keyboard_init(){
    enable_irq(KEYBOARD_IRQ);
}

/*
keyboard_handler()
inputs: none
outputs: none
handle the keyboard interrupt
*/
void do_keyboard_handler(){
    send_eoi(KEYBOARD_IRQ);
    cli();
    unsigned char c = getScancode();  //get the scancode and intepret later
    int index = (int)c;
    int i;

    switch (index){             //special keys manipultion
        case CAPSLOCK:
            CAPS_STATUS = CAPS_STATUS ^ 1;
            sti();
            return;
        case LEFTCONTROL:
            CTRL_STATUS = 1;
            sti();
            return;
        case LCONTROLRELEASE:
            CTRL_STATUS = 0;
            sti();
            return;
        case LEFTSHIFT:
        case RIGHTSHIFT:
            SHIFT_STATUS = 1;
            sti();
            return;
        case BACKSPACE:
            if(buffer_idx[curr_term]>0){
                buffer_idx[curr_term]--;

                backspace_handler();
            }
            sti();
            return;
        case LSHIFTRELEASE:
        case RSHIFTRELEASE:
            SHIFT_STATUS = 0;
            sti();
            return;
        case LEFTALT:
            ALT_STATUS = 1;
            sti();
            return;
        case LEFTALTRELEASE:
            ALT_STATUS = 0;
            sti();
            return;
    }

    if(ALT_STATUS ==1){
        switch (index){
            case FONE:
                terminal_switch(0);
                sti();
                return;
            case FTWO:
                terminal_switch(1);
                sti();
                return;
            case FTHREE:
                terminal_switch(2);
                sti();
                return;
            }
    }

    if(CTRL_STATUS == 1 && index == LPRESS){

        clear();
        //buffer_idx = 0;
        for(i = 0; i < buffer_idx[curr_term]; i++){
            set_keyboard_flag();
            putc(input_buffer[curr_term][i]);
        }
        sti();
        return;
    }

    if(index == ENTER){   //0x1c is the enter scancode
        newline_mark[curr_term] = 1;
        //input_buffer[curr_term][buffer_idx[curr_term]] = '\n';
        //buffer_idx[curr_term]++;
        set_keyboard_flag();

        sti();
        return;
    }

    //127 is the biggest buffer_idx can be,  0x3B is the largest code we should print
    if((buffer_idx[curr_term] >= BUFFER_SIZE - 1) || index > LARGEST_SCAN_CODE || CTRL_STATUS){
        sti();
        return;
    }



    if(CAPS_STATUS==0 && SHIFT_STATUS == 0){    //the four statuses of the keyboard
        input_buffer[curr_term][buffer_idx[curr_term]]=scantable[index];
        set_keyboard_flag();
        putc(scantable[index]);
    }
    else if(CAPS_STATUS==1 && SHIFT_STATUS == 0){
        input_buffer[curr_term][buffer_idx[curr_term]]=captable[index];
        set_keyboard_flag();
        putc(captable[index]);
    }
    else if(CAPS_STATUS==0 && SHIFT_STATUS == 1){
        input_buffer[curr_term][buffer_idx[curr_term]]=shifttable[index];
        set_keyboard_flag();
        putc(shifttable[index]);
    }
    else{
        input_buffer[curr_term][buffer_idx[curr_term]]=shiftcap[index];
        set_keyboard_flag();
        putc(shiftcap[index]);
    }
    buffer_idx[curr_term]++;
    sti();
}

/*
terminal_init()
inputs: none
outputs: none
description: initializes cursor position, clears the screen, and maps
             the pages for 3 terminals
*/
void terminal_init(){
    int i = 0;
    for(i = 0; i < TERMINAL_CNT; i++){

        screenx[i] = 0;
        screeny[i] = 0;
        clear();
        map_video_page(i);
    }
}

/*
terminal_open
description: initialize the terminal
inputs: none
outputs: none
return value: 0
*/
int terminal_open(const uint8_t* filename){
    buffer_idx[curr_term] = 0;
    return 0;
}
/*
terminal_close
description: close the terminal
inputs: none
outputs: none
return 0;
*/
int terminal_close(){
    return 0;
}

/*
terminal_read
description: read from keyboard buffer to buf
inputs: buf -- buffer to be written to
        size -- number of bytes to be written
outputs: none
return value: number of bytes read
*/
int terminal_read(unsigned char* buf, int size){
    int ret;
    while(!newline_mark[curr_scheduled_terminal]){};
    newline_mark[curr_scheduled_terminal] = 0;
    putc('\n');
    input_buffer[curr_term][buffer_idx[curr_term]] = '\n';
    buffer_idx[curr_term]++;
  /*  if(input_buffer[curr_term][0] == '\n'){
        input_buffer[curr_term][0] = '\0';
        return 0;
    }*/

    if(size <= buffer_idx[curr_term]){
        memcpy(buf, input_buffer[curr_term], size);
        buffer_idx[curr_term] = 0;
        return size;
    }
    else{
        memcpy(buf, input_buffer[curr_term], buffer_idx[curr_term]);
        ret = buffer_idx[curr_term];
        buffer_idx[curr_term] = 0;
        return ret;
    }
}

/*
terminal_write
description: write to the terminal from the buffer
inputs: buf -- buffer that stores string to be written
        size -- number of bytes to be written
outputs: none
return value: number of bytes actually written
*/
int terminal_write(unsigned char* buf, int size){
    //int buf_size = strlen(buf);
    int i = 0;
    for(i = 0; i < size; i++){
        if(buf[i] == 0)
            continue;
        putc(buf[i]);
    }
    return i;
}

/*
terminal_switch
description: switch pages when switching terminals
inputs: term: the terminal to switch to
outputs: none
return value: number of bytes actually written
*/
int terminal_switch(int term){
    /* upon terminal switching, change the current video memory page to the corresponding inactive terminal page */
    page_table_video[curr_term + 1] = (VIDEO_MEMORY_ADDRESS+(curr_term+1)*PAGE_4KB)|PRW;
    TLB_flushing();
    /* copy the video memory content to the inactive terminal page */
    memcpy(video_memory_terminal[curr_term], (void *)VIDEO_MEMORY_ADDRESS, VIDEO_SIZE);
    /* in the case of vidmap address is available, change it to its corresponding inactive page */
    if(launch_flags[0] && launch_flags[1] && launch_flags[2]){
        int old_pid = get_terminal_running_pid(curr_term);
        PCB_t* old_pcb = get_pcb_by_pid(old_pid);
        if(old_pcb->mapped_video_addr != NULL){
            page_table_video[4+curr_term] = (VIDEO_MEMORY_ADDRESS+(curr_term+1)*PAGE_4KB)|PRWU;
        }
    }
    /* copy the new terminal content to video memory, and map its inactive page to video memory */
    curr_term = term;
    memcpy((void *)VIDEO_MEMORY_ADDRESS,video_memory_terminal[curr_term], VIDEO_SIZE);
    page_table_video[term + 1] = VIDEO_MEMORY_ADDRESS|PRW;
    TLB_flushing();
    set_screen_cursor();
    /* in the case of vidmap address is available, map its page to video memory */
    if(launch_flags[0] && launch_flags[1] && launch_flags[2]){
        int curr_pid = get_terminal_running_pid(term);
        PCB_t* curr_pcb = get_pcb_by_pid(curr_pid);
        if(curr_pcb->mapped_video_addr != NULL){
            page_table_video[4+term] = VIDEO_MEMORY_ADDRESS|PRWU;
        }
    }
    return 0;
}

/*
map_video_page
description: map the pages for 3 terminals right after the video mem page
inputs: i:terminal idx
outputs: none
return value: none
*/
void map_video_page(int i){
    int a;
    page_directory[(PROGRAM_ADDR+PAGE_4MB)/PAGE_4MB] = ((uint32_t)page_table_video)|PRW;
    page_table_video[i+1] = (VIDEO_MEMORY_ADDRESS+(i+1)*PAGE_4KB)|PRW;
    video_memory_terminal[i] = (uint8_t*)((PROGRAM_ADDR+PAGE_4MB)+(i+1)*PAGE_4KB);
    /* clear the screen page */
    for(a = 0; a < VIDEO_SIZE; a++){
        video_memory_terminal[i][a] = ' ';
    }
}

/*
get_display_terminal
description: helper function to get the current displayed term
inputs: none
outputs: none
return value: current displayed terminal
*/
unsigned int get_display_terminal(){
    return curr_term;
}

/*
getScancode()
inputs: none
outputs: none
get the scancode for the keyboard interrupt
*/
unsigned char getScancode() {
    do {
        unsigned char temp = inb(KEYBOARD_PORT1);
        if(temp!=0) {   //keep trying if we get null
            return temp;               //return when it is valid
        }
    } while(1);
}
