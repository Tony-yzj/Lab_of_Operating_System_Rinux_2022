// arch/riscv/kernel/vm.c
#include "defs.h"
#include "mm.h"
#include "printk.h"

/* early_pgtbl: 用于 setup_vm 进行 1GB 的 映射。 */
unsigned long  early_pgtbl[512] __attribute__((__aligned__(0x1000)));

void setup_vm(void) {
    /* 
    1. 由于是进行 1GB 的映射 这里不需要使用多级页表 
    2. 将 va 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
        high bit 可以忽略
        中间9 bit 作为 early_pgtbl 的 index
        低 30 bit 作为 页内偏移 这里注意到 30 = 9 + 9 + 12， 即我们只使用根页表， 根页表的每个 entry 都对应 1GB 的区域。 
    3. Page Table Entry 的权限 V | R | W | X 位设置为 1
    */
   //0x1000对齐， 9位vpn对应44位ppn和10位flag
   //0x80000000 映射--0x80000000（第一次映射）table[index=start>>30] = PPN (PA >> 12) + flag 0000001111
   early_pgtbl[PHY_START>>30 & 0x1FF] = (PHY_START >> 12) << 10 | 15;
   //0x80000000 映射--0xffffffe000000000（第二次映射）table[index=start>>30] = PPN (PA >> 12) + flag 0000001111
   early_pgtbl[VM_START>>30 & 0x1FF] = (PHY_START >> 12) << 10 | 15;

   printk("...set up vm done.\n");
}

/* swapper_pg_dir: kernel pagetable 根目录， 在 setup_vm_final 进行映射。 */
unsigned long  swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));

// extern uint64 _stext;
// extern uint64 _srodata;
// extern uint64 _sdata;
extern void _stext();
extern void _srodata();
extern void _sdata();
extern void _skernel();


void setup_vm_final(void) {
    memset(swapper_pg_dir, 0x0, PGSIZE);

    uint64 text = ((uint64)&_srodata - (uint64)&_stext)/PGSIZE;
    uint64 rodata = ((uint64)&_sdata - (uint64)&_srodata)/PGSIZE;
    uint64 other = PHY_SIZE/PGSIZE - text -rodata;
    
    // No OpenSBI mapping required

    // mapping kernel text X|-|R|V
    create_mapping(swapper_pg_dir, (uint64)&_stext, (uint64)&_stext - PA2VA_OFFSET, text, 0b1011);
    printk("set up [text] is done...\n");
    // mapping kernel rodata -|-|R|V
    create_mapping(swapper_pg_dir, (uint64)&_srodata, (uint64)&_srodata - PA2VA_OFFSET, rodata, 0b0011);
    printk("set up [rodata] is done...\n");
    // mapping other memory -|W|R|V
    create_mapping(swapper_pg_dir, (uint64)&_sdata, (uint64)&_sdata - PA2VA_OFFSET, other, 0b0111);
    printk("set up [other] is done...\n");
    
    
    // set satp with swapper_pg_dir

    uint64 ppn = ((uint64)(swapper_pg_dir)-PA2VA_OFFSET >> 12 | 0x8000000000000000);
    asm volatile("csrw satp, %0"::"r"(ppn):);

    // flush TLB
    asm volatile("sfence.vma zero, zero");
    printk("...set up vm final done.\n");

    // printk("test for this lab...\n");
    // unsigned long int *p = 0xffffffe000202000;
    // unsigned long int raw = *p;
    // *p = raw;
    return;
}


/* 创建多级页表映射关系 */
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm) {
    /*
    pgtbl 为根页表的基地址
    va, pa 为需要映射的虚拟地址、物理地址
    sz 为映射的大小
    perm 为映射的权限

    创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
    可以使用 V bit 来判断页表项是否存在
    */
    while(sz)
    {
        uint64 vpn[3];
        uint64 *level2, *level3;
        //从虚拟地址获取vpn
        vpn[0] = va >> 12 & 0x1FF;
        vpn[1] = va >> 21 & 0x1FF;
        vpn[2] = va >> 30 & 0x1FF;

        //二级页表操作
        if(pgtbl[vpn[2]] & 0x01)
        {
            //获取PPN对应的物理地址
            level2 = (uint64 *)(((pgtbl[vpn[2]] >> 10) << 12) + PA2VA_OFFSET);
            // printk("%lx", (((pgtbl[vpn[2]] >> 10) << 12) + PA2VA_OFFSET));
        }
        else
        {
            //若没有这一页项，则使用kalloc分配
            level2 = (uint64*)kalloc();
            //新建页表项存储 PPN+valid bit=1
            uint64 pa2 = (uint64)level2-PA2VA_OFFSET;
            pgtbl[vpn[2]] = (((pa2) >> 12) << 10) | 1;
        }

        //三级页表操作
        if(level2[vpn[1]] & 0x01)
        {
            //获取PPN对应的物理地址
            level3 = (uint64 *)(((level2[vpn[1]] >> 10) << 12) + PA2VA_OFFSET);
        }
        else
        {
            //若没有这一页项，则使用kalloc分配
            level3 = (uint64*)kalloc();
            //新建页表项存储 PPN+valid bit=1
            uint64 pa3 = (uint64)level3-PA2VA_OFFSET;
            level2[vpn[1]] = (((pa3) >> 12) << 10) | 1;
        }

        //三级页表映射到物理地址
        level3[vpn[0]] = (pa >> 12 << 10) | perm;

        
        va += PGSIZE;
        pa += PGSIZE;
        sz--;
    }
   
}