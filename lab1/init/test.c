#include "print.h"
#include "defs.h"

// Please do not modify

void test() {
    //检查能否输出负数
    puts("test for negative integer: ");
    puti(-100);
    //判断能否输出0
    puts("\ntest for zero: ");
    puti(0);
    //读sstatus寄存器
    unsigned long csr_sstatus = csr_read(sstatus);
    puts("\nthe sstatus register is: ");
    puti(csr_read(sstatus));
    //写sscratch寄存器
    csr_write(sscratch, 10);
    puts("\nthe sscratch register is: ");
    puti(csr_read(sscratch));
    while (1);
}
