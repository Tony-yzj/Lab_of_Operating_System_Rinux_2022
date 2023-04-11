.section .text.entry
.align 2
.globl _traps
_traps:
# YOUR CODE HERE
# -----------
# 用户栈和内核栈的切换判断
    #t0用于暂存需要判断的值，先保存，不然上下文难以完全恢复
    addi sp, sp, -8
    sd t0, 0(sp)
    #读现在的sscratch寄存器
    csrr t0, sscratch
    #若sscratch为0，表明是kernel-->kernel，则不需要切换，跳转到原来的上下文切换
    beq t0, x0, noswitch_start
    #否则是user-->kernel，需要切换，则从sscratch中载入内核栈，将用户栈存入sscratch中
    csrw sscratch, sp
    #将栈针赋值为内核sp
    mv sp, t0    
    
noswitch_start:
    #将sscratch如栈，并不管此前是用户态还是内核态，都置0(这样使得代码重用性更好)
    addi sp, sp, -8;
    csrr t0, sscratch
    sd t0, 0(sp)
    csrw sscratch, x0
# 1. save 32 registers and sepc to stack
    #sscratch上下文保存寄存器，暂时存储sp
    # csrw sscratch, sp
    #操作sp
    addi sp, sp, -34*8
    #存x0～x31 sp暂时不动
    sd x0, 0*8(sp)
    sd x1, 1*8(sp)
    sd x2, 2*8(sp)
    sd x3, 3*8(sp)
    sd x4, 4*8(sp)
    sd x5, 5*8(sp)
    sd x6, 6*8(sp)
    sd x7, 7*8(sp)
    sd x8, 8*8(sp)
    sd x9, 9*8(sp)
    sd x10, 10*8(sp)
    sd x11, 11*8(sp)
    sd x12, 12*8(sp)
    sd x13, 13*8(sp)
    sd x14, 14*8(sp)
    sd x15, 15*8(sp)
    sd x16, 16*8(sp)
    sd x17, 17*8(sp)
    sd x18, 18*8(sp)
    sd x19, 19*8(sp)
    sd x20, 20*8(sp)
    sd x21, 21*8(sp)
    sd x22, 22*8(sp)
    sd x23, 23*8(sp)
    sd x24, 24*8(sp)
    sd x25, 25*8(sp)
    sd x26, 26*8(sp)
    sd x27, 27*8(sp)
    sd x28, 28*8(sp)
    sd x29, 29*8(sp)
    sd x30, 30*8(sp)
    sd x31, 31*8(sp)
    #存储sepc
    csrr t0, sepc
    sd t0, 32*8(sp)
    #存储sstatus--用于pt_regs
    csrr t0, sstatus
    sd t0, 33*8(sp)
    # csrr t0, sscratch
    # csrw sscratch, x0
    # sd t0, 2*8(sp)

# -----------
# 2. call trap_handler
    csrr a0, scause
    csrr a1, sepc
    #pt_regs
    mv a2, sp
    call trap_handler

# -----------
# 3. restore sepc and 32 registers (x2(sp) should be restore last) from stack
    #sstatus寄存器其实没有必要再恢复，允许其修改
    ld t0, 33*8(sp)
    csrw sstatus, t0
    ld t0, 32*8(sp)
    csrw sepc, t0

    ld x31, 31*8(sp)
    ld x30, 30*8(sp)
    ld x29, 29*8(sp)
    ld x28, 28*8(sp)
    ld x27, 27*8(sp)
    ld x26, 26*8(sp)
    ld x25, 25*8(sp)
    ld x24, 24*8(sp)
    ld x23, 23*8(sp)
    ld x22, 22*8(sp)
    ld x21, 21*8(sp)
    ld x20, 20*8(sp)
    ld x19, 19*8(sp)
    ld x18, 18*8(sp)
    ld x17, 17*8(sp)
    ld x16, 16*8(sp)
    ld x15, 15*8(sp)
    ld x14, 14*8(sp)
    ld x13, 13*8(sp)
    ld x12, 12*8(sp)
    ld x11, 11*8(sp)
    ld x10, 10*8(sp)
    ld x9, 9*8(sp)
    ld x8, 8*8(sp)
    ld x7, 7*8(sp)
    ld x6, 6*8(sp)
    ld x5, 5*8(sp)
    ld x4, 4*8(sp)
    ld x3, 3*8(sp)
    ld x1, 1*8(sp)
    ld x0, 0*8(sp)

    ld x2, 2*8(sp)
    #因为sscratch会用于切换用户栈和内核栈，所以稍微修改一下此处逻辑
    addi sp, sp, 34*8

    #恢复原来压入栈中的sscratch
    ld t0, 0(sp)
    addi sp, sp, 8
    #若此前进入时，sscratch为0，则表示是kernel-->kernel
    beq t0, x0, noswitch_end
    #否则为kernel-->user，需要恢复user栈的sp
    csrw sscratch, sp
    mv sp, t0
    
noswitch_end:
    ld t0, 0(sp)
    addi sp, sp, 8
# -----------
# 4. return from trap
    sret
# -----------

.global __dummy
__dummy:
    #切换sscratch和sp
    csrr t1, sscratch
    csrw sscratch, sp
    mv sp, t1
    # la t0, dummy
    # csrw sepc, t0
    sret

.globl __switch_to
__switch_to:
    # save state to prev process
    # YOUR CODE HERE
    sd ra, 5*8(a0)
    sd sp, 6*8(a0)
    sd s0, 7*8(a0)
    sd s1, 8*8(a0)
    sd s2, 9*8(a0)
    sd s3, 10*8(a0)
    sd s4, 11*8(a0)
    sd s5, 12*8(a0)
    sd s6, 13*8(a0)
    sd s7, 14*8(a0)
    sd s8, 15*8(a0)
    sd s9, 16*8(a0)
    sd s10, 17*8(a0)
    sd s11, 18*8(a0)
    csrr t0, sepc
    sd t0, 19*8(a0)
    csrr t0, sstatus
    sd t0, 20*8(a0)
    csrr t0, sscratch
    sd t0, 21*8(a0)
    # restore state from next process
    # YOUR CODE HERE
    ld ra, 5*8(a1)
    ld sp, 6*8(a1)
    ld s0, 7*8(a1)
    ld s1, 8*8(a1)
    ld s2, 9*8(a1)
    ld s3, 10*8(a1)
    ld s4, 11*8(a1)
    ld s5, 12*8(a1)
    ld s6, 13*8(a1)
    ld s7, 14*8(a1)
    ld s8, 15*8(a1)
    ld s9, 16*8(a1)
    ld s10, 17*8(a1)
    ld s11, 18*8(a1)
    ld t0, 19*8(a1)
    csrw sepc, t0
    ld t0, 20*8(a1)
    csrw sstatus, t0
    ld t0, 21*8(a1)
    csrw sscratch, t0
    
    #切换页表
    ld t1, 22*8(a1)
    srl t1, t1, 12
    li t2, 0x8000000000000000
    or t1, t1, t2
    csrw satp, t1

    # flush tlb
    sfence.vma zero, zero

    #flush icache
    fence.i

    ret