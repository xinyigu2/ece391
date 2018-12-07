#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "exce_hdl.h"
#include "keyboard.h"
#include "rtc.h"
#include "paging.h"
#include "file_sys.h"
#include "i8259.h"
#include "systemcall.h"
#include "malloc.h"
#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;
	int i;
	int * ptr;
	ptr = NULL;
	int result = PASS;
	for (i = 0; i < 32; ++i){
		if ((idt[i].offset_31_16 << 16) +
			(idt[i].offset_15_00) == 0){
			assertion_failure();
			result = FAIL;
		}
	}
	//i = *ptr;
	//while(1);
	return result;
}

/* IDT Test - Example
 *
 * Asserts that all IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 */
int idt_test_final(){
	TEST_HEADER;
	int i;
	int * ptr;
	ptr = NULL;
	int result = PASS;
	for (i = 0; i < 256; ++i){   //256 is the table size
		if ((idt[i].offset_31_16 << 16) +  //16 is the bits to shift
			(idt[i].offset_15_00) == 0){
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}


/* Checkpoint 1 tests */

/*rtc_test
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * initialize the rtc to test it
 */

 void rtc_test(int32_t freq){
	void* buf;
	//int32_t nbytes = 4;
	if(rtc_open() == -1){
		printf("rtc open failure\n");
		return;
	}
	if(rtc_write((uint8_t*)(&freq), 4) == -1){ // comment this to test rtc_open
		printf("rtc write failure\n");
		//return;
	}
	//cannot test rtc read yet
	while(!rtc_read(buf, 5)){
		putc('1');
	};
	if(rtc_close() == -1){
		printf("rtc close failure\n");
		return;
	}
}


/*exception_test_1
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * test divide by zero
 */
void exception_test_1(){
	int i=5;   //random number
	int j=0;
	i = i / j; //divide by zero
}


/*exception_test_2
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * test invalid opcode
 */
void exception_test_2(){

	__asm__("mov %cr6, %eax");  //cr6 is a made up register, wrong opcode testing

}


/*paging_test
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * test paging
 */
void paging_test(){
	unsigned char * kernel_addr;
	unsigned char * kernel_ending_addr;
	unsigned char * video_begin;
	kernel_addr = (unsigned char *)KERNEL_ADDRESS;
	kernel_ending_addr = (unsigned char *)KERNEL_ENDING;
	video_begin = (unsigned char *)VIDEO_MEMORY_ADDRESS;
	//printf("Value at 0: %d\n", *((unsigned char *)0));
	printf("Value at beginning of kernel: %d\n", *kernel_addr);
	printf("Value at end of kernel: %d\n", *(kernel_ending_addr - 1));
	//printf("Value right after kernel: %d\n", *kernel_ending_addr);
	//printf("Value right before kernel: %d\n", *(kernel_addr - 1));
	printf("Value at beginning of video memory: %d\n", *video_begin);
	printf("Value at the end of video memory: %d\n", *(video_begin + VIDEO_MEMORY_LENGTH - 1));
	printf("Value right after video memory: %d\n", *(video_begin + VIDEO_MEMORY_LENGTH));
}
/*PIC INPUT TEST
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * test PIC
 */
 void PIC_TESTS(){
	 int invalid_irq = 16;//invalid irq number
	 printf("start PIC invalid irq INPUT TEST, invalid IRQ number %d\n",invalid_irq);

	 printf("enable IRQ number %d\n",invalid_irq);
	 enable_irq(invalid_irq);// enable invalid irq
	 if(is_invalid_irq_input == 1){
		 printf("invalid enable irq number\n");
		 i8259_init();//initialize i8259 again because of invalid input
	 }

	 printf("disable IRQ number %d\n",invalid_irq);
	 disable_irq(invalid_irq);//disable invalid irq
	 if(is_invalid_irq_input == 1){
		 printf("invalid disable irq number\n");
		 i8259_init();//initialize i8259 again because of invalid input
	 }

	 printf("send eoi to IRQ number %d\n",invalid_irq);
	 send_eoi(invalid_irq); //send eoi to invalid irq
	 if(is_invalid_irq_input == 1){
		printf("invalid send eoi irq number\n");
		i8259_init();//initialize i8259 again because of invalid input
	}

	printf("finished PIC tests, initialize keyboard IRQ \n");
	keyboard_init();
	//rtc_init();
 }
/* Checkpoint 2 tests */


/*terminal_write_test
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * test terminal_write
 */
void terminal_write_test(){
	//terminal_open();
	unsigned char buf[20]="rrrrsdadasdarrrr\n";   //a random buffer to write to
	terminal_write(buf,1);
	terminal_close();
}


/*terminal_write_test_overflow
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * test terminal_write
 */
 /*
void terminal_write_test_overflow(){
	//terminal_open();
	unsigned char buf[200]="rrrrdjflksdjfsdjflksjfldskfjdslkfjdslkfjslfjlsdkfjsldkfjsdlkfjsdlkfjlskdjflskdjflksjflksfjlkdsjflkjsdlfjslfjldskfjsldkfjlskfjsdlkjkdsjfslkfjjdslssdkjskdjdadasdarrrr";   //a random buffer to write to
	terminal_write(buf,1);
	terminal_close();
}
*/
/*
void dir_read_test_ls(){
	int fd, cnt;
	uint8_t buf[33];
	terminal_open(".");
	if (-1 == (fd = dir_open((uint8_t*)"."))) {
		printf("directory open failed\n");
		return ;
	}
	while (0 != (cnt = dir_read (buf, 32))) {
		if (-1 == cnt) {
			printf("directory entry read failed\n");
			return ;
		}
		buf[cnt] = '\n';
		if (-1 == terminal_write(buf, cnt + 1))
			return ;
	}
}
*/


/*terminal_read_test
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * test terminal_read
 */
void terminal_read_test(){
	//terminal_open();
	int i = 0;
	unsigned char buf[20];   //20 is the buffer size
	int c =terminal_read(buf, 20);
	for(i=0; i<c-1; i++){
		putc(buf[i]);    //print to see if the buffer is well read
	}
	terminal_close();

}


/* Checkpoint 3 tests */




/* Checkpoint 4 tests */
/* Checkpoint 5 tests */
int32_t malloc_test(uint32_t bytes){
	int32_t current =malloc_(bytes);
	if(current == -1){
		printf("fail to malloc %d\n" , bytes);
	}else{
		printf("returned address:%#x\n",current );
	}
	return current;
}

void free_test(uint32_t address){
	int32_t current =free_(address);
	if(current == -1){
		printf("fail to free %#x\n" , address);
	}else{
		printf("free space at %#x\n" , address);
	}
}




/* Test suite entry point */
void launch_tests(){
	free_test(749190); // free non-existing space at beginging

	int32_t addr0 = malloc_test(PAGE_4MB); // free two individual page and then free
	int32_t addr1 = malloc_test(PAGE_4KB);
	free_test(addr0);
	free_test(addr1);


	free_test(749190); // free non-existing space in between

  	int32_t addr2 =	malloc_test(PAGE_4KB/2); // handle internel fragmentation
	int32_t addr3 = malloc_test(PAGE_4KB/4);
	free_test(addr2);
	free_test(addr3);

	int32_t addr4 =	malloc_test(PAGE_4MB/3); // handle internel fragmentation
	int32_t addr5 = malloc_test(PAGE_4MB/3);
	free_test(addr5);
	free_test(addr4);

	malloc_test(2147483647); // malloc tool large page

	free_test(addr0);
	free_test(addr1);
	free_test(addr2);// free the page which is already freed
	free_test(addr3);


}
