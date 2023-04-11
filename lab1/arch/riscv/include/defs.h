#ifndef _DEFS_H
#define _DEFS_H

#include "types.h"

#define csr_read(csr)                               \
({                                                  \
    register uint64 __v;                            \
    asm volatile ("csrr %0," #csr                   \
                    : "=r" (__v) :                  \
                    : "memory");                    \
    __v;                                            \
})//me finished
//=r 表示输出，会修改绑定的变量_v

#define csr_write(csr, val)                         \
({                                                  \
    uint64 __v = (uint64)(val);                     \
    asm volatile ("csrw " #csr ", %0"               \
                    : : "r" (__v)                   \
                    : "memory");                    \
})

#endif
