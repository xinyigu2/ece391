#include "exce_hdl.h"
#include "lib.h"
#include "systemcall.h"

/* exception handler for divide error */
void divide_error(){
    printf("exception 0: divide error\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for debug error */
void debug(){
    printf("exception 1: debug\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for non-maskable interrupt */
void nmi(){
    printf("exception 2: nmi\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for breakpoint error */
void int3(){
    printf("exception 3: breakpoint\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for overflow */
void overflow(){
    printf("exception 4: overflow\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for bounds error */
void bounds(){
    printf("exception 5: bounds\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for invalid opcode */
void invalid_op(){
    printf("exception 6: invalid opcode\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for device not available */
void device_not_available(){
    printf("exception 7: device not available\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for double fault */
void doublefault_fn(){
    printf("exception 8: double fault fn\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for coprocessor segment overrun */
void coprocessor_segment_overrun(){
    printf("exception 9: coprocessor segment overrun\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for invalid TSS */
void invalid_TSS(){
    printf("exception 10: invalid TSS\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for segment not present */
void segment_not_present(){
    printf("exception 11: segment not present\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for stack segment fault */
void stack_segment(){
    printf("exception 12: stack segment fault\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for general protection fault */
void general_protection(){
    printf("exception 13: general protection\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for page fault */
void page_fault(){
    printf("exception 14: page fault\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for Intel-reserved */
void Intel_reserved(){
    printf("exception 15: Intel-reserved\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for coprocessor error */
void coprocessor_error(){
    printf("exception 16: coprocessor error\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for alignment check */
void alignment_check(){
    printf("exception 17: alignment check\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for machine check */
void machine_check(){
    printf("exception 18: machine check\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
/* exception handler for SIMD floating point error */
void simd_coprocessor_error(){
    printf("exception 19: SIMD floating point\n");
    /* terminate current process */
    ece391_halt((uint8_t)EXCEPTION_HALT);
}
