.extern start_kernel

    .section .text.init
    .globl _start
_start:
    la sp, boot_stack_top  #栈针指向栈的顶部，负向增长
    call mm_init
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

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 # 4K size <-- change to your stack size

    .globl boot_stack_top
boot_stack_top: