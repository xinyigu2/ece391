#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "idt.h"
#include "exce_hdl.h"
#include "keyboard.h"
#include "mouse.h"
#include "rtc.h"
#include "scheduler.h"

/*
default_interrupt_handler()
inputs: none
outputs: none
just a default interrupt handler used for initialization
*/
void default_interrupt_handler(){
    printf("default interrupt\n");
}

/*
init_idt()
inputs: none
outputs: none
initialize the idt in a naive manner
*/
void init_idt(){
    lidt(idt_desc_ptr);
    int index = 0;
    for(index = 0; index < TAB_NUM; index ++){
        // the reserved bits for the interrupt gate should be 01100 for interrupt gate
        idt[index].reserved4 = 0;
        idt[index].reserved3 = 0;
        idt[index].reserved2 = 1;
        idt[index].reserved1 = 1;
        idt[index].reserved0 = 0;
        idt[index].size = 1;
        idt[index].dpl = 0;      //priority level is 0 for now
        idt[index].present = 1;
        idt[index].seg_selector = KERNEL_CS;
        SET_IDT_ENTRY(idt[index], default_interrupt_handler);  //a default intr handler for init
    }
    SET_IDT_ENTRY(idt[0], divide_error);   // set all the exception entries according to the number , from 1 to 19
    SET_IDT_ENTRY(idt[1], debug);
    SET_IDT_ENTRY(idt[2], nmi);
    SET_IDT_ENTRY(idt[3], int3);
    SET_IDT_ENTRY(idt[4], overflow);
    SET_IDT_ENTRY(idt[5], bounds);
    SET_IDT_ENTRY(idt[6], invalid_op);
    SET_IDT_ENTRY(idt[7], device_not_available);
    SET_IDT_ENTRY(idt[8], doublefault_fn);
    SET_IDT_ENTRY(idt[9], coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[10], invalid_TSS);
    SET_IDT_ENTRY(idt[11], segment_not_present);
    SET_IDT_ENTRY(idt[12], stack_segment);
    SET_IDT_ENTRY(idt[13], general_protection);
    SET_IDT_ENTRY(idt[14], page_fault);
    SET_IDT_ENTRY(idt[15], Intel_reserved);
    SET_IDT_ENTRY(idt[16], coprocessor_error);
    SET_IDT_ENTRY(idt[17], alignment_check);
    SET_IDT_ENTRY(idt[18], machine_check);
    SET_IDT_ENTRY(idt[19], simd_coprocessor_error);
    // the reserved bits for the interrupt gate should be 01110 for trap gate
    idt[SYS_CALL_ENTRY].reserved4 = 0;
    idt[SYS_CALL_ENTRY].reserved3 = 1;
    idt[SYS_CALL_ENTRY].reserved2 = 1;
    idt[SYS_CALL_ENTRY].reserved1 = 1;
    idt[SYS_CALL_ENTRY].reserved0 = 0;
    idt[SYS_CALL_ENTRY].size = 1;
    idt[SYS_CALL_ENTRY].dpl = 3;      //priority level is 3 for syscall
    idt[SYS_CALL_ENTRY].present = 1;
    idt[SYS_CALL_ENTRY].seg_selector = KERNEL_CS;
    SET_IDT_ENTRY(idt[RTC_ENTRY], rtc_handler);   //set the interrupt entry for the keyboard and rtc
    SET_IDT_ENTRY(idt[KEYBOARD_ENTRY], keyboard_handler);
    SET_IDT_ENTRY(idt[MOUSE_ENTRY], mouse_handler);
    SET_IDT_ENTRY(idt[SYS_CALL_ENTRY], system_call_handler);
    SET_IDT_ENTRY(idt[PIT_ENTRY], pit_handler);
};
