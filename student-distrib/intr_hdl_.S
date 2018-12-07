.global keyboard_handler, rtc_handler, pit_handler,mouse_handler

/* keyboard_handler: keyboard interrupt handler mapped to IDT,
					 the actual body is in intr_hdl.c
 */
keyboard_handler:
	pushl %eax				# save all register
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
    movl %esp, %eax			# linkage to C function
    call do_keyboard_handler
	popl %ebx				# restore all registers
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
    iret					# interrupt return


/* rtc_handler: rtc interrupt handler mapped to IDT,
				 the actual body is in intr_hdl.c
 */

rtc_handler:
    pushl %eax				# save all register
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    movl %esp, %eax			# linkage to C function
    call do_rtc_handler
    popl %ebx				# restore all registers
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %eax
    iret					# interrupt return

/* pit_handler: pit interrupt handler mapped to IDT,
				 the actual body is in scheduler.c
 */

pit_handler:
    pushl %eax				# save all register
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    movl %esp, %eax			# linkage to C function
    call do_pit_handler
    popl %ebx				# restore all registers
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %eax
    iret					# interrupt return

	/* mouse_handler: mouse interrupt handler mapped to IDT,
	 */

	mouse_handler:
	    pushl %eax				# save all register
	    pushl %ebp
	    pushl %edi
	    pushl %esi
	    pushl %edx
	    pushl %ecx
	    pushl %ebx
	    movl %esp, %eax			# linkage to C function
	    call do_mouse_handler
	    popl %ebx				# restore all registers
	    popl %ecx
	    popl %edx
	    popl %esi
	    popl %edi
	    popl %ebp
	    popl %eax
	    iret					# interrupt return


.end
