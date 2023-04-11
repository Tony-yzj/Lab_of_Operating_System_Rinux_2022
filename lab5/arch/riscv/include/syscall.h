#include "types.h"
#include "proc.h"
#include "printk.h"

void sys_write(unsigned int fd, const char* buf, uint64 count);
unsigned long sys_getpid();