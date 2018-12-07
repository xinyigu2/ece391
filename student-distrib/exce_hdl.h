#ifndef EXCEPTION_HANDLER
#define EXCEPTION_HANDLER
#define EXCEPTION_HALT 99    //Unique number to distinguish between halt from exception and normal halt

void divide_error();
void debug();
void nmi();
void int3();
void overflow();
void bounds();
void invalid_op();
void device_not_available();
void doublefault_fn();
void coprocessor_segment_overrun();
void invalid_TSS();
void segment_not_present();
void stack_segment();
void general_protection();
void page_fault();
void Intel_reserved();
void coprocessor_error();
void alignment_check();
void machine_check();
void simd_coprocessor_error();

#endif
