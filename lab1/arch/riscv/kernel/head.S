.extern start_kernel

    .section .text.entry
    .globl _start
_start:
    # ------------------
    # - your code here -
    # ------------------
    la sp, boot_stack_top  #栈针指向栈的顶部，负向增长
    jal ra, start_kernel #跳转到start_kernel

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 # 4K size <-- change to your stack size

    .globl boot_stack_top
boot_stack_top: