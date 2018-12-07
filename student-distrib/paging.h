#ifndef PAGING_H
#define PAGING_H
#include "types.h"

#define RW 0x02
#define PRW 0x03
#define PRWMbS 0x83
#define PRWMbU 0x87
#define PRWU 0x07
//0x83 = 10000011
#define BYTES_TO_ALIGN_TO 4096
#define PAGE_SIZE 1024
#define VIDEO_MEMORY_ADDRESS 0xB8000
#define KERNEL_ADDRESS 0x400000
#define KERNEL_ENDING 0x800000
#define PAGE_LENGTH 12
#define VIDEO_MEMORY_LENGTH 4096
#define PAGE_4MB 0x400000
#define PAGE_4KB 0x1000
#define MB_140 0x8C00000
#define PROGRAM_ADDR 0x8000000
#define PROGRAM_ADDR_OFFSET 0x48000


extern void paging_init();
extern void page_directory_init();
extern void page_table_init();
extern void task_paging_setup(unsigned char *mem_addr, int task_idx);
extern void enable_paging(uint32_t page_directory);
extern void TLB_flushing();

/* page directory and table for memory */
uint32_t page_directory[PAGE_SIZE] __attribute__((aligned(BYTES_TO_ALIGN_TO)));
uint32_t page_table[PAGE_SIZE] __attribute__((aligned(BYTES_TO_ALIGN_TO)));
uint32_t page_table_video[PAGE_SIZE] __attribute__((aligned(BYTES_TO_ALIGN_TO)));


#endif
