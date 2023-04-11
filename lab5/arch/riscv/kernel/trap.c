// trap.c 
#include "printk.h"
#include "syscall.h"
struct pt_regs
{
    uint64 x[32];
    uint64 sepc;
    uint64 sstatus;
};

extern void clock_set_next_event();
void trap_handler(unsigned long scause, unsigned long sepc, struct pt_regs *regs) {
    // 通过 `scause` 判断trap类型
    // 如果是interrupt 判断是否是timer interrupt
    // 如果是timer interrupt 则打印输出相关信息, 并通过 `clock_set_next_event()` 设置下一次时钟中断
    // `clock_set_next_event()` 见 4.5 节
    // 其他interrupt / exception 可以直接忽略
    
    // YOUR CODE HERE
    if(scause & 0x8000000000000000)
    {
        //interrupt
        if(scause & 0x8000000000000005)
        {
            // printk("[S] Supervisor Mode Timer Interrupt\n");
            clock_set_next_event();
            do_timer();
        }
    }
    else
    {
        //exception
        if(scause & 0x0000000000000008)
        {
            if(regs->x[17] == 64)
            {
                sys_write(regs->x[10],(char*)regs->x[11], regs->x[12]);
            }
            else if(regs->x[17] == 172)
            {
                regs->x[10] = sys_getpid();
            }
            regs->sepc = regs->sepc + (uint64)4;
        }
    }

}