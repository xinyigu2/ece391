.global enable_paging, TLB_flushing


/* enable_paging
 * DESCRIPTION: pass the address of directory and enables paging
 * INPUT/OUTPUT: none
 * SIDE EFFECT: change cr0, cr3 and cr4 to enable paging
 */
enable_paging:
    pushl   %ebp                    # callee-save begins
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi                    # callee-save ends

    movl    8(%ebp), %esi           # esi stores paging directory
    movl    %esi, %cr3              # stores paging directory to PDBR(cr3)

    movl    %cr4, %esi              # change bit 0 of cr4 to 1 to enable 4MB pages
    orl     $0x00000010, %esi       # 0x00000010 = 00000000000000000000000000010000
    movl    %esi, %cr4

    movl    %cr0,%esi               # change bit 31 of cr0 to 1 to enable page
    orl     $0x80000000, %esi       # 0x80000000 = 10000000000000000000000000000000
    movl    %esi, %cr0

    popl    %edi                    # callee-restore begins
    popl    %esi
    popl    %ebx                    # callee-restore ends
    leave
    ret

/* TLB_flushing
 * DESCRIPTION: flush tlb by simply refreshing cr3
 * INPUT/OUTPUT: none
 * SIDE EFFECT: flush tlb
 */
TLB_flushing:
    pushl   %ebp                    # callee-save begins
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi                    # callee-save ends

    movl    %cr3, %ebx              # put PDBR(cr3) to EBX
    movl    %ebx, %cr3              # move EBX back to cr3

    popl    %edi                    # callee-restore begins
    popl    %esi
    popl    %ebx                    # callee-restore ends
    leave
    ret


.end
