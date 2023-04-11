// trap.c 
#include "printk.h"
void trap_handler(unsigned long scause, unsigned long sepc) {
    // 通过 `scause` 判断trap类型
    // 如果是interrupt 判断是否是timer interrupt
    // 如果是timer interrupt 则打印输出相关信息, 并通过 `clock_set_next_event()` 设置下一次时钟中断
    // `clock_set_next_event()` 见 4.5 节
    // 其他interrupt / exception 可以直接忽略
    
    // YOUR CODE HERE
    //scause寄存器  第一位是1--中断，后面表示中断类型
    if(scause & 0x8000000000000000)
    {
        //interrupt
        //如果是时钟中断类型
        if(scause & 0x8000000000000005)
        {
            //打印信息
            printk("[S] Supervisor Mode Timer Interrupt\n");
            //设置下一次时钟中断
            clock_set_next_event();
        }
    }
    //是异常的话，此处处理（暂未实现）
    else
    {
        //exception
    }

}