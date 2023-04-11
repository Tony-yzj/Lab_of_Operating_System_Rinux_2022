#ifndef _SBI_H
#define _SBI_H

#define SBI_SETTIME 0x0
#define SBI_PUTCHAR 0x1
#define SBI_GETCHAR 0x2
#define SBI_SHUTDOWN 0x2

#include "types.h"

struct sbiret {
	long error;
	long value;
};

struct sbiret sbi_ecall(int ext, int fid, uint64 arg0,
			            uint64 arg1, uint64 arg2,
			            uint64 arg3, uint64 arg4,
			            uint64 arg5);

void sbi_set_timer(uint64 time);
void sbi_console_putchar(char ch);
char sbi_console_getchar(char *ch);
void sbi_shutdown();
 
#endif
