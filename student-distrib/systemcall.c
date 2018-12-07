#include "systemcall.h"
#include "x86_desc.h"
#include "paging.h"
#include "file_sys.h"
#include "lib.h"
#include "scheduler.h"
#include "tests.h"

#define HIGHEST_FREQ 1024
#define EMPTY -1
#define TERMINAL_CNT 3
#define VIDMAP_OFFSET 4

jumptable_t table_rtc={rtc_read, rtc_write, rtc_open, rtc_close};
jumptable_t table_dir={dir_read, dir_write, dir_open, dir_close};
jumptable_t table_file={file_read, file_write, file_open, file_close};
jumptable_t table_stdin = {terminal_read, terminal_write, terminal_open, terminal_close};
jumptable_t table_stdout = {terminal_read, terminal_write, terminal_open, terminal_close};

uint32_t pid_map[MAX_PROCESS] = {0,0,0,0,0,0};//at most 6 processes
extern int curr_term;
extern int curr_scheduled_terminal;
extern char *video_mem_array[TERMINAL_CNT];

/*
ece391_halt
input: uint8_t status
output: none
side effect: handle the halt system call
*/

int32_t ece391_halt (uint8_t status){
    int halting_pid, curr_pid;
    PCB_t* halting_pcb;
    int i; // shut all fd down

    /* find pid to terminate */
    for(i = MAX_PROCESS - 1; i >= 0; i--){
        if(curr_terminal_pid[curr_scheduled_terminal][i] != EMPTY){
            halting_pid = curr_terminal_pid[curr_scheduled_terminal][i];
            curr_terminal_pid[curr_scheduled_terminal][i] = EMPTY;
            break;
        }
    }
    pid_map[halting_pid] = 0;
    halting_pcb = get_pcb_by_pid(halting_pid);

    /* close all files */
    for(i = 2; i < MAX_FILE ;i++){
        if(halting_pcb->opened_files[i].flags == 1){
            (void)ece391_close(i);
        }
    }
    /* clear argument */
    for(i = 0; i < ARG_LENGTH; i++)
        halting_pcb->argument[i] = '\0';


    curr_pid = halting_pcb->parent_pid;
    tss.esp0 = halting_pcb->parent_tss_esp0;

    //restore paging;
    task_paging_setup((unsigned char *)PROGRAM_ADDR, curr_pid);
    TLB_flushing();


    /* if no process is currently running, start another shell */
    if((!pid_map[0]) || (!pid_map[1]) || (!pid_map[2])){
        printf("last process halted\n");
        ece391_execute((uint8_t *)"shell");
    }
    // go to excute using assembly linkage
    // use a special number to determine if halt is called from user or exception handler
    if(status == EXCEPTION_HALT){
        // return 256 in case of exception
        asm volatile(
            "movl $256, %%eax;"
            "movl %%ebp, %%esp;"
            "movl %0, %%ebp;"
            "jmp execute_label;"
            :
            :"r"((halting_pcb->parent_ebp))
            : "eax"
        );
    }else{
        asm volatile(
            "movzbl %%BL, %%eax;"
            "movl %%ebp, %%esp;"
            "movl %0, %%ebp;"
            "jmp execute_label;"
            :
            :"r"((halting_pcb->parent_ebp))
            : "eax"
        );
    }
    return 0;
}

/*
ece391_execute
input: uint8_t command
output: none
side effect: handle the execute system call
*/
int32_t ece391_execute (const uint8_t* command){
    cli();
    uint32_t starting_addr;
    uint8_t name[ARG_LENGTH];
    uint8_t args[ARG_LENGTH];
    int32_t new_pid = -1;
    uint32_t i, arg_cnt, name_cnt = 0;
    int executing_pid = get_terminal_running_pid(curr_scheduled_terminal);
    if(command == NULL)
        return -1;
    /* parse name from the input command */
    for(i = 0; i < strlen((int8_t*)command); i++){
        if(command[i] == '\n')
            continue;
        if(command[i] == ' ')
            break;
        name[name_cnt] = command[i];
        name_cnt++;
    }
    name[name_cnt] = '\0';

    /* parse the argument from input command */
    arg_cnt = 0;
    while(i < strlen((int8_t*)command)){
        if(command[i] == ' ' && arg_cnt == 0){
            i++;
            continue;
        }
        else{
            args[arg_cnt] = command[i];
            arg_cnt++;
            i++;
        }
    }
    args[arg_cnt] = '\0';

    /* get starting address of the program */
    if(-1 == (starting_addr = program_info(name))){
        sti();
        return -1;
    }

    /* look for available process slot */
    for(i = 0; i < MAX_PROCESS; i++){
        if(pid_map[i] == 0){
            new_pid = i;
            break;
        }
    }
    if(new_pid == -1){
        printf("full process\n");
        sti();
        return 2;
    }
    /* upper bound of 2 tasks are set for the current checkpoint */
    pid_map[new_pid] = 1;

    /* update terminal pid */
    for(i = 0; i < MAX_PROCESS; i++){
        if(curr_terminal_pid[curr_term][i] == -1){
            curr_terminal_pid[curr_term][i] = new_pid;
            //printf("found empty pid at %d, %d\n", curr_term, i);
            break;
        }
    }


    /* store information of the current process into the pcb of the new process */
    PCB_t* new_pcb = get_pcb_by_pid(new_pid);

    //PCB_t* curr_pcb = get_pcb_by_pid(executing_pid);
    if(new_pid != 0)
        new_pcb->parent_pid = executing_pid;
    else
        new_pcb->parent_pid = -1;

    new_pcb->pid = new_pid;
    new_pcb->opened_files[0].jumptable = &table_stdin;
    new_pcb->opened_files[1].jumptable = &table_stdout;
    new_pcb->opened_files[0].flags = 1;
    new_pcb->opened_files[1].flags = 1;

    for(i = 2 ; i < MAX_FILE ; i++){
      new_pcb->opened_files[i].flags = 0;
    }
      for(i = 0; i < MAX_FILE ; i++){
      new_pcb->opened_files[i].file_position = 0;
      new_pcb->opened_files[i].inode = 0;
    }

    new_pcb->parent_tss_esp0 = tss.esp0;
    new_pcb->mapped_video_addr = NULL;

    /* copy argument to pid */
    strcpy((int8_t *)new_pcb->argument, (int8_t *)args);


    /* set up paging for the new program before loading it, flush
     * the TLB, and load the program
     */
    task_paging_setup((unsigned char *)PROGRAM_ADDR, new_pid);
    TLB_flushing();

    if(-1 == program_loader(name))
        return -1;

    //printf("executing pid: %d\n", new_pid);
    /* set up ss0 and esp0 for tss */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = BOTTOM - new_pid*OFFSET;   //7 is the offset for 6 processes

    /* save ebp for the current process */
    asm volatile (
        "movl %%ebp, %0;"
        :"=&r"((new_pcb->parent_ebp))
    );


    /* push iret context to stack. */
    /* 0x002B represents user data segment.
     * 0x0023 represents user code segment.
     * 0x83FFFFF is the user level stack pointer under
     * the end of the program page.
     * Input starting_addr will be pushed to %eip after
     * iret is executed.
     */
    asm volatile ("           \
        mov $0x002B, %%ax   \n\
        mov %%ax, %%ds       \n\
        mov %%ax, %%es       \n\
        mov %%ax, %%fs       \n\
        mov %%ax, %%gs       \n\
        mov %%esp, %%eax     \n\
        pushl $0x002B      \n\
        push $0x83FFFFF    \n\
        pushf              \n\
        popl %%eax         \n\
        orl $0x200, %%eax   \n\
        pushl %%eax          \n\
        pushl $0x0023      \n\
        pushl %0           \n\
        iret               \n\
        execute_label:          \n\
        leave                   \n\
        ret                     \n\
        "                       \
        :                     \
        : "r"(starting_addr)  \
        :"eax"          \
    );

    return 0;

}

/*
ece391_read
input: int32_t fd, void* buf, int32_t nbytes
output: none
side effect: handle the read system call
*/
int32_t ece391_read (int32_t fd, void* buf, int32_t nbytes){
    /* check if fd is valid */
    if(fd < 0 ||  fd > (MAX_FILE-1))
        return -1;

    /* stdout should be write-only */
    if(fd == 1)
        return -1;

    int executing_pid = get_terminal_running_pid(curr_scheduled_terminal);
    PCB_t* curr_pcb = get_pcb_by_pid(executing_pid);
    /* check whether the give fd is occupied */
    if(curr_pcb->opened_files[fd].flags == 0)
        return -1;

    if(buf == NULL)
        return -1;

    curr_pcb->current_fd = fd;

    return curr_pcb->opened_files[fd].jumptable->read(buf, nbytes);

}

/*
ece391_write
input: int32_t fd, void* buf, int32_t nbytes
output: none
side effect: handle the write system call
*/
int32_t ece391_write (int32_t fd, void* buf, int32_t nbytes){
    /* check if fd is valid */
    if(fd < 0 || fd > (MAX_FILE-1))
        return -1;
    /* stdin should be read-only */
    if(fd == 0)
        return -1;

    int executing_pid = get_terminal_running_pid(curr_scheduled_terminal);
    PCB_t* curr_pcb = get_pcb_by_pid(executing_pid);

    if(buf == NULL)
        return -1;

    curr_pcb->current_fd = fd;
    /* check whether the give fd is occupied */
    if(curr_pcb->opened_files[fd].flags == 0)
        return -1;

    return curr_pcb->opened_files[fd].jumptable->write(buf, nbytes);
}



/*
ece391_open
input: const uint8_t* filename
output: none
side effect: handle the open system call
*/
int32_t ece391_open (const uint8_t* filename){
    dentry_t dir_entry;
    if(read_dentry_by_name(filename, &dir_entry) == -1)//not a valid filename
        return -1;
    int i;
    int32_t fd = 0;

    int executing_pid = get_terminal_running_pid(curr_scheduled_terminal);
    PCB_t* curr_pcb = get_pcb_by_pid(executing_pid);
    for(i = 2; i < MAX_FILE; i++){
        if(curr_pcb->opened_files[i].flags == 0){
            fd = i;
            break;
        }
    }
    if(fd == 0)//  no descriptors are free
        return -1;

    curr_pcb->opened_files[fd].flags = 1; //set to using
    /* assign the corresponding function jump table, inode and position
     * depending on the file type
     */
    switch (dir_entry.filetype)
    {
        case RTC_TYPE:
    		if (-1 == rtc_open(filename)){
    			return -1; //fail to open
            }
            curr_pcb->opened_files[fd].jumptable = &table_rtc;
            curr_pcb->opened_files[fd].inode = 0;
            curr_pcb->opened_files[fd].file_position = HIGHEST_FREQ;   // we use file_position to keep record of the frequency
    		break;
    	case DIR_TYPE:
    		if (-1 == dir_open(filename)){
    			return -1; //fail to open
            }
            curr_pcb->opened_files[fd].jumptable = &table_dir;
            curr_pcb->opened_files[fd].inode = 0;
            curr_pcb->opened_files[fd].file_position = 0;
            break;
    	case FILE_TYPE:
    		if (-1 == file_open(filename)){
    			return -1; //fail to open
            }
            curr_pcb->opened_files[fd].jumptable = &table_file;
            curr_pcb->opened_files[fd].inode = dir_entry.inode_num;
            curr_pcb->opened_files[fd].file_position = 0;
    		break;
    }
	return fd;

}

/*
ece391_close
input: int32_t fd
output: none
side effect: handle the close system call
*/
int32_t ece391_close (int32_t fd){
    /* check if fd is valid */
    if(fd < 0 ||  fd > (MAX_FILE-1))
        return -1;

    if(fd == 0 || fd == 1)
        return -1;

    int executing_pid = get_terminal_running_pid(curr_scheduled_terminal);
    PCB_t* curr_pcb = get_pcb_by_pid(executing_pid);
    /* check whether the give fd is occupied */
    if(curr_pcb->opened_files[fd].flags == 0)
        return -1;

    curr_pcb->opened_files[fd].flags = 0;

    return curr_pcb->opened_files[fd].jumptable->close();

}

/*
ece391_getargs
input: uint8_t* buf -- buffer to store arguments
       int32_t nbytes -- number of bytes to read
output: none
return value: 0 if successful, -1 otherwise
side effect: return argument back to user program
*/
int32_t ece391_getargs (uint8_t* buf, int32_t nbytes){
    if(buf == NULL)
        return -1;

    int i, empty_flag = 1;
    int executing_pid = get_terminal_running_pid(curr_scheduled_terminal);
    PCB_t* curr_pcb = get_pcb_by_pid(executing_pid);

    if(nbytes < strlen((int8_t *)curr_pcb->argument))
        return -1;

    /* check whether argument is empty */
    for(i = 0; i < strlen((int8_t *)curr_pcb->argument); i++){
        if(curr_pcb->argument[i] != '\0' || curr_pcb->argument[i] != ' '){
            empty_flag = 0;
            break;
        }
    }
    if(empty_flag)
        return -1;

    strcpy((int8_t *)buf, (int8_t *)curr_pcb->argument);

    return 0;
}

/*
ece391_vidmap
input: uint8_t** screen_start -- pointer to the address get passed
                                 back to user level pointing to video
                                 memory
output: none
return value: 0 if successful, -1 otherwise
side effect: return a virtual memory address pointing to video memory
*/
int32_t ece391_vidmap (uint8_t** screen_start){
    if(screen_start == NULL)
        return -1;
    /* check whether the input address falls within the range of the kernel page */
    if(screen_start >= (uint8_t **)KERNEL_ADDRESS && screen_start < (uint8_t **)KERNEL_ENDING)
        return -1;

    /* map a 4kB page at PROGRAM_ADDR+PAGE_4MB = 132MB to the user-level video mem.
     * the physical memory of vidmap pages are the 4th, 5th and 6th pages right
     * right after video memory page.
     */
    page_directory[(PROGRAM_ADDR+PAGE_4MB)/PAGE_4MB] = ((uint32_t)page_table_video)|PRWU;
    page_table_video[VIDMAP_OFFSET + curr_scheduled_terminal] = (VIDEO_MEMORY_ADDRESS)|PRWU;
    *screen_start = (uint8_t *)(PROGRAM_ADDR+PAGE_4MB + (VIDMAP_OFFSET + curr_scheduled_terminal)*PAGE_4KB);

    int executing_pid = get_terminal_running_pid(curr_scheduled_terminal);
    PCB_t* curr_pcb = get_pcb_by_pid(executing_pid);

    curr_pcb->mapped_video_addr = PROGRAM_ADDR+PAGE_4MB + (VIDMAP_OFFSET + curr_scheduled_terminal)*PAGE_4KB;
    return 0;
}

int32_t ece391_set_handler (int32_t signum, void* handler){return -1;}
int32_t ece391_sigreturn (void){return -1;}


/*
 get_pcb_by_pid(uint32_t pid)
input: uint32_t pid
output: the pcb pointer
side effect: return the pcb pointer given the pid
*/
PCB_t * get_pcb_by_pid(uint32_t pid){
    //kernel stack is 8kb(OFFSET)each
    //the first user-level program (the shell) should be loaded at physical 8 MB
    return (PCB_t *)(BOTTOM - (pid + 1) * OFFSET);
}
