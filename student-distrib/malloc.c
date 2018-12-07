/* Include the sbrk function */
#include "malloc.h"



// typedef struct mem_control_block {
//  int is_available;
//  int size;
// }mcb_t;

int blockidx;
mp_t space_map[MAX_BLOCKS];


int flag = 0;

void space_map_init(){
  //initialize the firs three big block of free space;
  space_map[0].starting_address = 4096;
  space_map[0].size = VIDEO_MEMORY_ADDRESS ;
  space_map[0].page_ending = space_map[0].starting_address + space_map[0].size;
  space_map[0].is_available = 1;
  space_map[0].previous_idx = -1;
  space_map[0].next_idx = 1;
  space_map[0].dir_idx = -1;
  space_map[0].table_idx = -1;
  space_map[0].cache = 0;
  space_map[0].cache_address = -1;



  space_map[1].starting_address = VIDEO_MEMORY_ADDRESS + PAGE_4KB;
  space_map[1].size = KERNEL_ADDRESS - space_map[1].starting_address;
  space_map[1].page_ending = space_map[1].starting_address + space_map[1].size;
  space_map[1].is_available = 1;
  space_map[1].previous_idx = 0;
  space_map[1].next_idx = 2;
  space_map[1].dir_idx = -1;
  space_map[1].table_idx = -1;
  space_map[1].cache = 0;
  space_map[0].cache_address = -1;


  // 256 * PAGE_4MB is the ending of the physical memory we assume
  // 6 * PAGE_4MB is the space for tasks

  space_map[2].starting_address = KERNEL_ENDING + 6 * PAGE_4MB;
  space_map[2].size = PAGE_4MB*256 - space_map[2].starting_address; //PAGE_4MB*256 = ohysical ending
  space_map[2].page_ending = space_map[2].starting_address + space_map[2].size;
  space_map[2].is_available = 1;
  space_map[2].previous_idx = 1;
  space_map[2].next_idx = -1;
  space_map[2].dir_idx = -1;
  space_map[2].table_idx = -1;
  space_map[2].cache = 0;
  space_map[0].cache_address = -1;



  blockidx = 2;
  flag = 1;
}

// malloc space which is larger than 4mb
int32_t MAP_4MB(uint32_t bytes){
  int32_t i,j,diridx;
  // find the avaliable space
  for(i = 0; i <= blockidx; i++){
    if(space_map[i].size >= bytes && space_map[i].is_available == 1){
      blockidx++;
      space_map[blockidx].size = bytes;
      space_map[blockidx].starting_address = space_map[i].starting_address;
      space_map[blockidx].page_ending = space_map[blockidx].starting_address + PAGE_4MB* (int)((bytes-1)/PAGE_4MB+1);
      if(space_map[blockidx].page_ending > PAGE_4MB*256){
        return -1;
      }//out of range
      space_map[blockidx].is_available = 0;
      space_map[blockidx].previous_idx = space_map[i].previous_idx;
      space_map[blockidx].next_idx = i;
      space_map[blockidx].cache = bytes % PAGE_4MB;
      // get a new uesed space block
      space_map[i].size -= PAGE_4MB* (int)(bytes/PAGE_4MB);
      space_map[i].starting_address =space_map[blockidx].page_ending;
      space_map[i].previous_idx = blockidx;
      //update the neibouring block

      diridx = 0;
      //update page_directory
      for(j = 0; j < (int)((bytes-1)/PAGE_4MB+1); j++){
        while(page_directory[j+diridx]  != RW){ // PDE occupied
          diridx++;
        }
        page_directory[j+diridx] = (space_map[blockidx].starting_address + j*PAGE_4MB)|PRWMbS;
        TLB_flushing();
        diridx++;
      }
      space_map[blockidx].dir_idx = diridx-1;
      space_map[blockidx].table_idx = -1;
      return space_map[blockidx].starting_address;
    }
  }
  return -1;
}

// malloc space which is smaller than 4mb

int32_t MAP_4KB(uint32_t bytes){
  int32_t i, j, tableidx ;
// find the avaliable space
  for(i = 0; i <= blockidx; i++){
    if(space_map[i].size >= bytes && space_map[i].is_available == 1){
      blockidx++;
      space_map[blockidx].size = bytes;
      space_map[blockidx].starting_address = space_map[i].starting_address;
      space_map[blockidx].page_ending = space_map[blockidx].starting_address + PAGE_4KB* (int)((bytes-1)/PAGE_4KB+1);
      space_map[blockidx].is_available = 0;
      space_map[blockidx].previous_idx = space_map[i].previous_idx;
      space_map[blockidx].next_idx = i;
      space_map[blockidx].cache = bytes % PAGE_4KB;
      // get a new uesed space block
      space_map[i].size -= PAGE_4KB* (int)(bytes/PAGE_4KB);
      space_map[i].starting_address =space_map[blockidx].page_ending;
      space_map[i].previous_idx = blockidx;
      //update the neibouring block
      tableidx = 0;
      //update page_table
      for(j = 0; j < (int)((bytes-1)/PAGE_4KB+1); j++){
        while(page_table[j+tableidx] != RW){ // PTE occupied
          tableidx++;
        }
        page_table[j+tableidx] = (space_map[blockidx].starting_address + j*PAGE_4KB)|PRW;
        TLB_flushing();
        tableidx++;
      }
      space_map[blockidx].dir_idx = -1;
      space_map[blockidx].table_idx = tableidx-1;
      return space_map[blockidx].starting_address;
    }

  }
  return -1;
}






int32_t malloc_(uint32_t bytes){
  if(bytes <= 0){
    return -1;
  }// chack valid input
  if(flag==0){
    space_map_init();
  } // initialize if not
  int c ;
  //check whether cache have enough space for malloc
  for(c = 0; c <= blockidx; c++){
    if(space_map[c].cache >= bytes){
      space_map[c].cache_address = space_map[c].starting_address + space_map[c].size;
      space_map[c].cache = 0;
      return space_map[c].cache_address;
    }
  }
  if(bytes >= PAGE_4MB){
      return MAP_4MB(bytes);
  }else{
      return MAP_4KB(bytes);
  }
}


int32_t free_(uint32_t address){
  if(address < 0 || address > PAGE_4MB*256){
    return -1;
  }// chack valid input

  if(!flag){
    space_map_init();
  }// initialize if not
  uint32_t c,i,j;
  int32_t bytes;

  // check whether the space we need to free is in cache space
  // merge space blocks if possible
  for(c = 0; c <= blockidx ; c++){
    if(address == space_map[c].cache_address){
      space_map[c].cache = 0;
      space_map[c].cache_address = -1;
      bytes = space_map[c].size;
      if(space_map[c].is_available == 1){
        if(bytes>=PAGE_4MB){
          for(j = 0; j < (int)((bytes-1)/PAGE_4MB+1); j++){
            page_directory[space_map[c].dir_idx - j]  = RW ;
            TLB_flushing();
          }
          merge(c);
        }else{
          for(j = 0; j < (int)((bytes-1)/PAGE_4KB+1); j++){
            page_table[space_map[c].table_idx - j]  = RW;
            TLB_flushing();
          }
          merge(c);
        }
      }
      return 0;
    }
  }


  for(i = 0; i <= blockidx ; i++){
    if((address == space_map[i].starting_address)&&(space_map[i].is_available == 0)){
      bytes = space_map[i].size;
      if(space_map[i].cache == 0){
        if(bytes>=PAGE_4MB){
          for(j = 0; j < (int)((bytes-1)/PAGE_4MB+1); j++){
            page_directory[space_map[i].dir_idx - j]  = RW ;
            TLB_flushing();
          }
          merge(i);
        }else{
          for(j = 0; j < (int)((bytes-1)/PAGE_4KB+1); j++){
            page_table[space_map[i].table_idx - j]  = RW;
            TLB_flushing();
          }
          merge(i);
        }
      }
      space_map[i].is_available = 1;
      space_map[i].size -= space_map[i].cache;
      return 0;
    }
  }
  return -1; // no matching space block found
}

// merge funtion for memory blocks
void merge(uint32_t idx){
  int32_t prev,next;
  prev = space_map[idx].previous_idx;
  next = space_map[idx].next_idx;
  // check whther can be merged with previous block
  if(prev != -1){
    if(space_map[prev].is_available == 1 && space_map[prev].cache == 0){
      space_map[idx].starting_address = space_map[prev].starting_address;
      space_map[idx].size += space_map[prev].size;
      space_map[idx].is_available = 1;
      space_map[idx].previous_idx = space_map[prev].previous_idx;
      space_map[idx].cache = space_map[prev].cache;
      space_map[idx].cache_address = space_map[prev].cache_address;
      space_map[prev].is_available = -1;//abandon this block
      space_map[space_map[prev].previous_idx].next_idx = blockidx;
    }
  }
  // check whther can be merged with next block

  if(next != -1){
    if(space_map[next].is_available == 1){
      space_map[idx].page_ending = space_map[next].page_ending;
      space_map[idx].size += space_map[next].size;
      space_map[idx].is_available = 1;
      space_map[idx].next_idx = space_map[next].next_idx;
      space_map[idx].cache = space_map[next].cache;
      space_map[idx].cache_address = space_map[next].cache_address;
      space_map[next].is_available = -1;//abandon this block
      space_map[space_map[next].next_idx].previous_idx = blockidx;
    }
  }

}
