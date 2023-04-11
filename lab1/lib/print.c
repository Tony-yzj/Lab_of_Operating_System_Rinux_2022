#include "print.h"
#include "sbi.h"

void puts(char *s) {
    // unimplemented
    int cnt = 0;
    //遍历字符串数组
    while(s[cnt] != '\0')
    {
        //顺序输出
        sbi_console_putchar(s[cnt]);
        cnt++;
    }
}

void puti(int x) {
    //num暂存需要输出的值
    int p = 10, num = x;
    //处理负数情况
    if(x<0)
    {
        puts("-");
        num = -num;
    }
    //获取p--x的位数（以10^k表示）
    while(x/p)
    {
        p *= 10;
    }

    p /= 10;    //循环得到的p是高一位的，所以除以10
    while(p)
    {
        //输出从高位到低位
        sbi_console_putchar('0'+num/p);
        num %= p;
        p /= 10;
    }
}
