#ifndef SYSTEMCALL_H
#define SYSTEMCALL_H
#include "file_sys.h"
#include "rtc.h"
#include "keyboard.h"
#include "lib.h"

#define BOTTOM 0x800000    //pcb bottom
#define OFFSET 0x2000      //offset for a PCB
#define MAX_FILE 8         //fd array index
#define MAX_PROCESS 6      //maximum process number
#define EXCEPTION_HALT 99
#define ARG_LENGTH 128

/* PCB is stored at the end of kernel page. The first PCB is at the bottom
 * of kernel page, and the second is at 4kb above it, and so on.
 */
typedef struct {
    fd_t  opened_files[MAX_FILE];   // file array
    uint8_t argument[ARG_LENGTH];   // input argument associated
    uint32_t pid;                   // process ID
    uint32_t parent_pid;            // process ID of parent process
    uint32_t parent_ebp;            // %epb of parent process
    uint32_t parent_tss_esp0;       // tss' esp0 of parent process
    uint32_t scheduled_esp;         // saved %esp of current process before scheduling
    uint32_t scheduled_ebp;         // saved %ebp of current process before scheduling
    uint32_t scheduled_esp0;        // saved tss' esp0 of current process before scheduling
    uint32_t current_fd;            // current reading or writing file descriptor to pass to write or read function
    uint32_t mapped_video_addr;     // mapped user-level video mem address
} PCB_t;


int32_t ece391_halt (uint8_t status);
int32_t ece391_execute (const uint8_t* command);
int32_t ece391_read (int32_t fd, void* buf, int32_t nbytes);
int32_t ece391_write (int32_t fd, void* buf, int32_t nbytes);
int32_t ece391_open (const uint8_t* filename);
int32_t ece391_close (int32_t fd);
int32_t ece391_getargs (uint8_t* buf, int32_t nbytes);
int32_t ece391_vidmap (uint8_t** screen_start);
int32_t ece391_set_handler (int32_t signum, void* handler);
int32_t ece391_sigreturn (void);
PCB_t * get_pcb_by_pid(uint32_t pid);
int32_t get_curr_pid();




#endif
