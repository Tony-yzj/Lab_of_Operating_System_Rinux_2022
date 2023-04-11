#include "printk.h"
#include "defs.h"

// Please do not modify

void test() {
    // printk("the sstatus register is: ");
    // printk(csr_read(sstatus));
    // csr_write(sscratch, 10);
    // printk("\nthe sie register is: ");
    // printk(csr_read(sie));
    long int i = 0;
    while (1)
    {
        while(i<100000000)
            i++;
        printk("kernel is running!\n");
        i=0;
    }
}
