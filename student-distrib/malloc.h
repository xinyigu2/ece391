#ifndef MALLOC_H
#define MALLOC_H
#include "file_sys.h"
#include "systemcall.h"
#include "paging.h"
#include "lib.h"

#define PAGE_4KB 0x1000
#define MAX_BLOCKS 1024



typedef struct mp {
 uint32_t starting_address;
 uint32_t page_ending;
 uint32_t size;
 int32_t is_available;
 int32_t previous_idx;
 int32_t next_idx;
 uint32_t cache;
 int32_t cache_address;
 int32_t table_idx;
 int32_t dir_idx;
}mp_t;



void space_map_init();
int32_t MAP_4MB(uint32_t bytes);
int32_t MAP_4KB(uint32_t bytes);
int32_t malloc_(uint32_t bytes);
int32_t free_(uint32_t address);
void merge(uint32_t idx);

#endif
