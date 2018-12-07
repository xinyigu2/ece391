.global system_call_handler

/*
system_call_handler
description: assembly linkage from user-level program to system calls
             that saves all regs and check for validity of syscall number.
inputs: none
outputs: none
return value: non
*/
system_call_handler:


    #assembly linkage for th systemcall
    pushl %es
    pushl %ds
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushfl

    pushl %edx                      # push the three arguments
    pushl %ecx
    pushl %ebx

    cmpl $1, %eax                   # check the argument
    jl fail
    cmpl $10, %eax                  # 10 is the upper boundary
    jg fail

    subl $1, %eax                   # subtract 1 from the argument to adapt to the jumptable
    call *jumptable(,%eax,4)        # 4 is the size of a jumptable entry
    jmp valid

    fail:
 	movl $-1, %eax
    valid:
    popl %edx                       # pop the three arguments
    popl %ecx
    popl %ebx

    popfl
    popl %ebx				        # restore all registers
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
    popl %ds
    popl %es

    iret



    jumptable:
     .long ece391_halt,ece391_execute,ece391_read,ece391_write,ece391_open,ece391_close,ece391_getargs,ece391_vidmap,ece391_set_handler,ece391_sigreturn
