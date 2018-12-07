#ifndef FILE_SYS_H
#define FILE_SYS_H
#include "types.h"

//#define FS_ADDR 0x100000 /* to be validated */
#define FILENAME_LEN 32
#define DIR_ENTRY_NUM 63
#define STATISTICS_LEN 64
#define DENTRY_RESERVED 24
#define DATA_BLOCK_MAX 1023
#define RTC_TYPE 0
#define DIR_TYPE 1
#define FILE_TYPE 2

/* fops table for system call */
typedef struct {
   int32_t (*read)(uint8_t* buf, int32_t nbytes);
   int32_t (*write)(uint8_t* buf, int32_t nbytes);
   int32_t (*open)(const uint8_t* filename);
   int32_t (*close)();
} jumptable_t;



/* 64B dentry struct */
typedef struct __attribute__((packed)) dentry{
    int8_t filename[FILENAME_LEN];
    int32_t filetype;
    int32_t inode_num;
    int8_t reserved[DENTRY_RESERVED];    //24 bits reserved for 64B dir. entry
}dentry_t;

/* 4kB boot block struct */
typedef struct __attribute__((packed)) boot_block{
    int32_t dir_count;
    int32_t inode_count;
    int32_t data_count;
    int8_t reserved[52];    //52 bits reserved
    dentry_t direntries[DIR_ENTRY_NUM];
}boot_block_t;

/* 4kB inode struct */
typedef struct __attribute__((packed)) inode{
    int32_t length;
    int32_t data_block_num[DATA_BLOCK_MAX];
}inode_t;

/* 16B file descriptor struct */
typedef struct __attribute__((packed)) file_descriptor{
    jumptable_t* jumptable;
    int32_t inode;
    uint32_t file_position;
    int32_t flags;
}fd_t;
/* helper function for file system driver */
extern void file_system_init(boot_block_t* addr);
extern int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
/* helper for excecute system call */
extern int32_t program_info(const uint8_t* fname);
extern int32_t program_loader(const uint8_t* fname);
/* file drivers */
extern int32_t file_open(const uint8_t* fname);
extern int32_t file_close();
extern int32_t file_write(uint8_t* buf, int32_t count);
extern int32_t file_read(uint8_t* buf, int32_t count);
/* directory drivers */
extern int32_t dir_open(const uint8_t* dname);
extern int32_t dir_close();
extern int32_t dir_write(uint8_t* buf, int32_t count);
extern int32_t dir_read(uint8_t* buf, int32_t count);

#endif
