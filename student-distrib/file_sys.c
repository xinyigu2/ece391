#include "file_sys.h"
#include "lib.h"
#include "keyboard.h"
#include "paging.h"
#include "systemcall.h"
#include "scheduler.h"

#define MAGIC1 0x7F
#define MAGIC2 0x45
#define MAGIC3 0x4C
#define MAGIC4 0x46
#define STARTING_ADDR_BIT3 27
#define STARTING_ADDR_BIT2 26
#define STARTING_ADDR_BIT1 25
#define STARTING_ADDR_BIT0 24

static boot_block_t* file_system_addr;
static dentry_t current_file;
static dentry_t current_dir;
static int file_count;
extern int curr_scheduled_terminal;

/* file_system_init
 * DESCRIPTION: This function takes in the address of the file system
 *              in the image to initialize the file_system_addr variable
 *              shared with other functions in this driver
 * INPUT: addr -- address of the file system in the image
 * OUTPUT: none
 * RETURN VALUE: none
 */
void file_system_init(boot_block_t* addr){
    file_system_addr = addr;
}

/* read_dentry_by_name
 * DESCRIPTION: This function takes in a file name and a pointer to a
 *              pointer to a dentry struct, and fill the struct with
 *              the information of the file with the specified fname
 * INPUT: fname -- file name to be read
 *        dentry -- dentry struct to be filled
 * OUTPUT: none
 * RETURN VALUE: 0 if successful, and -1 if not
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    int i, j, match;
    for(i = 0; i < file_system_addr->dir_count; i++){
        match = 1;
        /* Traverse the list to find a matching name */
        for(j = 0; j < FILENAME_LEN; j++){
            if(fname[j] == 0){
                if((file_system_addr->direntries[i]).filename[j] != 0)
                match = 0;
                break;
            }
            if(fname[j] != (file_system_addr->direntries[i]).filename[j]){
                match = 0;
                break;
            }
        }
        /* if name exceeds length of 32, ignore */
        if(j == FILENAME_LEN && fname[j] != 0)
            continue;
        /* if name not matched, ignore */
        if(!match)
            continue;
        else{
            read_dentry_by_index(i, dentry);
            return 0;
        }
    }
    return -1;
}

/* read_dentry_by_index
 * DESCRIPTION: This function takes in a index and a pointer to a
 *              pointer to a dentry struct, and fill the struct with
 *              the information of the file with the specified index
 * INPUT: index -- file index to be read
 *        dentry -- dentry struct to be filled
 * OUTPUT: none
 * RETURN VALUE: 0 if successful, and -1 if not
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
    dentry_t target_dentry;
    int i;
    if(index < file_system_addr->dir_count){
        /* if index within range, fill dentry with corresponding info */
        target_dentry = file_system_addr->direntries[index];
        dentry->filetype = target_dentry.filetype;
        dentry->inode_num = target_dentry.inode_num;
        for(i = 0; i < FILENAME_LEN; i++)
            dentry->filename[i] = target_dentry.filename[i];
        for(i = 0; i < DENTRY_RESERVED; i++)
            dentry->reserved[i] = target_dentry.reserved[i];
        return 0;
    }
    return -1;
}

/* read_data
 * DESCRIPTION: This function takes in a inode, a buffer, a length
 *              and a offset, read a inode of a file by the length
 *              amount to the buffer beginning at the offset
 * INPUT: inode -- inode of the file to be read
 *        offset -- offset from the biginning of the file to the
 *                  beginning of the read
 *        buf -- buffer array to store the written bytes
 *        length -- number of bytes to be written
 * OUTPUT: none
 * RETURN VALUE: number of bytes not returned
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    if(inode >= file_system_addr->inode_count)
        return -1;
    if(offset/sizeof(inode_t) >= DATA_BLOCK_MAX || (offset + length)/sizeof(inode_t) >= DATA_BLOCK_MAX)
        return -1;

    inode_t *target_inode = (inode_t*)file_system_addr + 1 + inode;

    /* define first data beginning at the offset */
    int8_t *first_data = (int8_t*)file_system_addr + (file_system_addr->inode_count + 1
                        + target_inode->data_block_num[offset/sizeof(inode_t)])*sizeof(inode_t)
                        + offset%sizeof(inode_t);

    int ret = 0;
    int i;
    int8_t* curr_data = first_data;

    for(i = offset; i < offset + length; i++){
        /* check if reached end of file */
        if(i >= ((inode_t*)file_system_addr + 1 + inode)->length){
            return ret;
        }


        /* put value in current position in buffer */
        buf[i - offset] = *curr_data;
        ret++;

        /* if reached end of a data block, switch to the next block */
        if(i%sizeof(inode_t) == sizeof(inode_t) - 1)
            curr_data = (int8_t*)file_system_addr + (file_system_addr->inode_count + 1
                        + target_inode->data_block_num[(i + 1)/sizeof(inode_t)])*sizeof(inode_t);
        else
            curr_data++;
    }
    return ret;
}

/* program_info
 * DESCRIPTION: check whether the given file is an executable file
 * INPUT: fname -- file name
 * OUTPUT: none
 * RETURN VALUE: starting address of the program if executable, -1 if other wise
 */

int32_t program_info(const uint8_t* fname){
    dentry_t dentry;
    int32_t ret;
    uint8_t buf[50];
    ret = read_dentry_by_name(fname, &dentry);
    if(dentry.filetype != FILE_TYPE)
        return -1;
    if(0 != read_data(dentry.inode_num, 0, buf, 40)){
        //check the first 4 numbers for distinct magic number for executable
        if(buf[0] != MAGIC1 || buf[1] != MAGIC2 || buf[2] != MAGIC3 || buf[3] != MAGIC4){
            printf("Not executable\n");
            return -1;
        }
        else
            // pack the starting address into a single 32-bit integer
            return (buf[STARTING_ADDR_BIT3] << 24) + (buf[STARTING_ADDR_BIT2] << 16) + (buf[STARTING_ADDR_BIT1] << 8) + buf[STARTING_ADDR_BIT0];
    }
    return 0;
}

/*
program program_loader
input: const uint8_t* fname
output: none
side effects: load the program file in execute
 */
int32_t program_loader(const uint8_t* fname){
    uint8_t *addr = (uint8_t *)(PROGRAM_ADDR + PROGRAM_ADDR_OFFSET);       //program maps to virtual memory address 128MB
    int32_t cnt;
    int32_t content_cnt = 0;
    dentry_t dentry;
    uint8_t buf[50];
    if(-1 == read_dentry_by_name(fname, &dentry))
        return -1;
    /* start reading image and copy content to starting address */
    while (0 != (cnt = read_data(dentry.inode_num, content_cnt, buf, 32))) {
        if (-1 == cnt) {
            printf("directory entry read failed\n");
            return -1;
        }
        memcpy(addr, buf, cnt);
        addr += cnt;
        content_cnt += cnt;
    }
    return 0;
}

/* file_open
 * DESCRIPTION: This function takes in the name of a file, and set
 *              the global file to it to read from.
 * INPUT: fname -- name of the file to be opened
 * OUTPUT: none
 * RETURN VALUE: 0 if opened correctly, -1 otherwise
 */
int32_t file_open(const uint8_t* fname){
    int32_t ret;
    ret = read_dentry_by_name(fname, &current_file);
    if(current_file.filetype != FILE_TYPE){
        //printf("Can't open target. Target is not a file.\n");
        return -1;
    }
    return ret;
}

/* file_close
 * DESCRIPTION: This function undo the things done by file_open,
 *              by initializing the info in the struct to 0.
 * INPUT: none
 * OUTPUT: none
 * RETURN VALUE: 0
 */
int32_t file_close(){
    return 0;
}

/* file_write
 * DESCRIPTION: This function does nothing since it's a read-only
 *              file system.
 * INPUT: buf -- buffer to write to
 *        count -- number of bytes to be written
 * OUTPUT: none
 * RETURN VALUE: -1
 */
int32_t file_write(uint8_t* buf, int32_t count){
    return -1;
}

/* file_read
 * DESCRIPTION: This function reads from the currently opened file
 *              count number of bytes into buf.
 * INPUT: buf -- buffer to write to
 *        count -- number of bytes to be written
 * OUTPUT: none
 * RETURN VALUE: number of bytes written, or -1 if failed.
 */
int32_t file_read(uint8_t* buf, int32_t count){
    int fd, pid = get_terminal_running_pid(curr_scheduled_terminal);
    PCB_t* curr_pcb = get_pcb_by_pid(pid);
    fd = curr_pcb->current_fd;

    if(fd <= 1 || fd >= MAX_PROCESS)
        return -1;
    /* read data to buf */
    int ret = read_data(curr_pcb->opened_files[fd].inode, curr_pcb->opened_files[fd].file_position, buf, count);
    curr_pcb->opened_files[fd].file_position += ret;
    return ret;
}

/* dir_open
 * DESCRIPTION: This function takes in the name of a directory, and set
 *              the global directory to it to read from.
 * INPUT: dname -- name of the directory to be opened
 * OUTPUT: none
 * RETURN VALUE: 0 if opened correctly, -1 otherwise
 */
int32_t dir_open(const uint8_t* dname){
    int32_t ret;
    file_count = 0;
    ret = read_dentry_by_name(dname, &current_dir);
    if(current_dir.filetype != DIR_TYPE){
        //printf("Can't open target. Target is not a directory.\n");
        return -1;
    }
    return ret;
}

/* dir_close
 * DESCRIPTION: This function undo the things done by dir_open,
 *              by initializing the info in the struct to 0.
 * INPUT: none
 * OUTPUT: none
 * RETURN VALUE: 0
 */
int32_t dir_close(){
    return 0;
}

/* dir_write
 * DESCRIPTION: This function does nothing since it's a read-only
 *              file system.
 * INPUT: buf -- buffer to write to
 *        count -- number of bytes to be written
 * OUTPUT: none
 * RETURN VALUE: -1
 */
int32_t dir_write(uint8_t* buf, int32_t count){
    return -1;
}

/* dir_read
 * DESCRIPTION: This function prints out file names, types and sizes
 *              one at a time
 * INPUT: buf -- buffer to write to
 *        count -- number of bytes to be written
 * OUTPUT: none
 * RETURN VALUE: 0
 */
int32_t dir_read(uint8_t* buf, int32_t count){

    int ret = read_dentry_by_index(file_count, &current_dir);
    file_count++;
    if(ret == 0){
        memcpy(buf, current_dir.filename, count);
        return strlen(current_dir.filename) > FILENAME_LEN ? FILENAME_LEN : strlen(current_dir.filename);
    }
    else
        return 0;
}
