#include "syscall.h"

//64号系统调用--打印
void sys_write(unsigned int fd, const char* buf, uint64 count)
{
    //标准输出
    if(fd == 1)
    {
        //输出字符
        for(uint64 i=0; i<count; i++)
        {
            printk("%c",buf[i]);
        }
    }
    else
    {
        //尚未实现
    }
}

//172号系统调用--获取进程id
extern struct task_struct* current;


uint64 sys_getpid()
{
    return current->pid;
}