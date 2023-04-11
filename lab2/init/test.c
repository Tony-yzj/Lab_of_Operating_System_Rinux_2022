#include "printk.h"
#include "defs.h"
#include "sbi.h"

extern unsigned long get_cycles();
// Please do not modify

void test() {
    // printk("the sstatus register is: ");
    // printk(csr_read(sstatus));
    // csr_write(sscratch, 10);
    // printk("\nthe sie register is: ");
    // printk(csr_read(sie));
    unsigned long i = 0;
    while (1)
    {
        i = get_cycles();
        if(i % 5000000 == 0)
            printk("kernel is running!\n");
    }
}
