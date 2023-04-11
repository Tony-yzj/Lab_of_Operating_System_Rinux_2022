#include "types.h"
#include "sbi.h"


struct sbiret sbi_ecall(int ext, int fid, uint64 arg0,
			            uint64 arg1, uint64 arg2,
			            uint64 arg3, uint64 arg4,
			            uint64 arg5) 
{
    // unimplemented 

	//将变量a0-a7与寄存器a0-a7绑定(使用asm)
	register uint64 a0 asm ("a0"); 
	register uint64 a1 asm ("a1"); 
	register uint64 a2 asm ("a2"); 
	register uint64 a3 asm ("a3"); 
	register uint64 a4 asm ("a4"); 
	register uint64 a5 asm ("a5"); 
	register uint64 a6 asm ("a6");
	register uint64 a7 asm ("a7");
	//对寄存器赋值
	a0 = arg0, a1 = arg1, a2 = arg2, a3 = arg3, a4 = arg4, a5 = arg5;
	a6 = fid, a7 = ext;

	asm volatile (
		"ecall"
		:"+r" (a0), "+r" (a1)	//输出：+r 可读写
		:"r" (a0),"r" (a1),"r" (a2),"r" (a3),"r" (a4),"r" (a5),"r" (a6),"r" (a7) //输入
		:"memory"	//ecall可能需要操作a0-a7之外的内存
	);

	//将0，1通过sbiret传出函数
	struct sbiret temp;
	temp.error = a0;
	temp.value = a1;

	return temp;
}

//设置时钟相关寄存器
void sbi_set_timer(uint64 time)
{
	sbi_ecall(SBI_SETTIME, 0x0, time, 0, 0, 0, 0, 0);
}
//打印字符
void sbi_console_putchar(char ch)
{
	sbi_ecall(SBI_PUTCHAR, 0x0, (uint64)ch, 0, 0, 0, 0, 0);
}
//接收字符
char sbi_console_getchar(char* ch)
{
	sbi_ecall(SBI_GETCHAR, 0x0, (uint64)ch, 0, 0, 0, 0, 0);
}
//关机
void sbi_shutdown()
{
	sbi_ecall(SBI_SHUTDOWN, 0x0, 0, 0, 0, 0, 0, 0);
}