.extern start_kernel

    .section .text.init
    .globl _start
_start:
    la sp, boot_stack_top  #栈针指向栈的顶部，负向增长
    #初始化虚拟内存，设置satp
    call setup_vm
    call relocate
    #分配空间和初始化进程
    call mm_init
    call setup_vm_final
    call task_init
    # ------------------
    # - your code here -
    # ------------------
    #set stvec = _traps
    la t0, _traps
    csrw stvec, t0
    # ------------------
    # set sie[STIE] = 1
    csrr t1, sie
    ori t1, t1, 0x0020
    csrw sie, t1 
    # ------------------
    # set first time interrupt
    rdtime a0
    lui t1, (10000000>>12)
    addi t1, t1, 10000000&0xFFF
    add a0, a0, t1
    andi a1, a1, 0
    andi a2, a2, 0
    andi a3, a3, 0
    andi a4, a4, 0
    andi a5, a5, 0
    andi a6, a6, 0
    andi a7, a7, 0
    ecall
    # ------------------
    # set sstatus[SIE] = 1
    csrr t2, sstatus
    ori t2, t2, 0x0002
    csrw sstatus, t2
    # ------------------
    
    # ------------------
    # - your lab1 code -
    # ------------------
    
    jal ra, start_kernel #跳转到start_kernel

relocate:
    # set ra = ra + PA2VA_OFFSET
    # set sp = sp + PA2VA_OFFSET (If you have set the sp before)
    # li t0, PA2VA_OFFSET
    li t0, 0xFFFFFFDF80000000
    add ra, ra, t0
    add sp, sp, t0

    # set satp with early_pgtbl
    la t1, early_pgtbl
    srl t1, t1, 12
    li t2, 0x8000000000000000
    or t1, t1, t2
    csrw satp, t1
    
    # flush tlb
    sfence.vma zero, zero

    #flush icache
    fence.i

    ret

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 # 4K size <-- change to your stack size

    .globl boot_stack_top
boot_stack_top: