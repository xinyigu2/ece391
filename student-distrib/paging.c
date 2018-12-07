#include "paging.h"

/* page_directory_init
 * DESCRIPTION: initialize the page directory and set up the first entry to
 * page table and second entry to be kerney
 * INPUT/OUTPUT: none
 * SIDE EFFECT: map the kernel starting address and the coresponding RW and
 *              P value to page_directory[1], and the page table to page
 *              directory[0] for the 0 - 4MB portion.
 */
void page_directory_init(){
    int i;
    for(i = 0; i < PAGE_SIZE; i++){
      page_directory[i] = RW;
    }
    // set any unused pages to not present as well
    // initialize first 4MB in memory at 0
    // the ﬁrst 4 MB of memory should broken down into 4 kB pages
    page_directory[0] = ((uint32_t)page_table) | PRW;
    // initialize the kernel memory at 4MB
    page_directory[1] = KERNEL_ADDRESS|PRWMbS;
    //kernel is loaded at physical address 0x400000
}


/*
uint32_t present   : 1 ; //bit 0 ->set to 1 for  -> set to 1 for the first entry for pd//video memory
uint32_t Read      : 1 ; //read/write,bit 1  ->set to 1 for all
uint32_t User      : 1 ; //user/supervisor,bit2
uint32_t WT        : 1 ; //write through,bit3
uint32_t CD        : 1 ; //cache disbled,bit4
uint32_t Accessed  : 1 ; //bit5;
uint32_t Dirty     : 1 ; //bit6
uint32_t PAT       : 1 ; //Page Table Attribute Index bit 7
uint32_t G         : 1 ; //gloabal page, ignore here,bit 8
uint32_t ASPU      : 3 ; //Available for system programmer’s use,bit 9-11
uint32_t PBA       : 24; // Page Base Address,bit 12-31
*/

/* page_table_init
 * DESCRIPTION: initialize the page table and set up the video memory entry
 * INPUT/OUTPUT: none
 * SIDE EFFECT: initialize the page table and set up the video memory entry
 */
void page_table_init(){
    int i;
    for(i = 0; i < PAGE_SIZE; i++){
        //mark all field between 0-4MB as not present and enable writing
        page_table[i] = RW;
    }
    //setany unused pages to not present as well
    // VIDEO memory address  0xB8000
    //setting for video memory -> the first entry for pd
    int idx = VIDEO_MEMORY_ADDRESS >> PAGE_LENGTH; //SHIFT 12 BITS TO GET starting index for pt
    page_table[idx] = VIDEO_MEMORY_ADDRESS | PRW;
    //page_table[idx + 1] = ((idx + 1)<< PAGE_LENGTH) | PRW; // GET CORRESPONDING PAGE BASE ADDRESS
    //page_table[idx + 2] = ((idx + 2)<< PAGE_LENGTH)  | PRW;
    //page_table[idx + 3] = ((idx + 3)<< PAGE_LENGTH)  | PRW;
}

/* task_paging_setup
 * DESCRIPTION: initialize the page table and set up the video memory entry
 * INPUT/OUTPUT: mem_addr -- virtual memory to be mapped
 *               task_idx -- task index (0-based)
 * SIDE EFFECT: initialize the page table and set up the video memory entry
 */
void task_paging_setup(unsigned char *mem_addr, int task_idx){
    page_directory[(uint32_t)mem_addr/PAGE_4MB] = (KERNEL_ENDING + task_idx*PAGE_4MB) | PRWMbU;
}

/* paging_init
 * DESCRIPTION: initialize paging
 * INPUT/OUTPUT: none
 * SIDE EFFECT: set up paging of kernel and video memory
 */
void paging_init()
{
    // initialize entire page directory
    page_table_init();
    page_directory_init();
    enable_paging((uint32_t)page_directory);
}
