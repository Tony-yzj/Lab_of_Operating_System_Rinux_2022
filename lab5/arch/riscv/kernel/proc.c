//arch/riscv/kernel/proc.c
#include "proc.h"
#include "string.h"
#include "stdint.h"
#include "mm.h"
#include "elf.h"
extern void __dummy();
extern int printk(const char *, ...);
struct task_struct* idle;           // idle process
struct task_struct* current;        // 指向当前运行线程的 `task_struct`
struct task_struct* task[NR_TASKS]; // 线程数组, 所有的线程都保存在此
extern unsigned long  swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));
extern uint64 uapp_start;
extern uint64 uapp_end;

static uint64_t load_elf(struct task_struct* task) {
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)&uapp_start;
    uint64_t phdr_start = (uint64_t)ehdr + ehdr->e_phoff;
    int phdr_cnt = ehdr->e_phnum;
    Elf64_Phdr* phdr;
    int load_phdr_cnt = 0;
    for (int i = 0; i < phdr_cnt; i++) {
        phdr = (Elf64_Phdr*)(phdr_start + sizeof(Elf64_Phdr) * i);
        if (phdr->p_type == PT_LOAD) {
        // copy the program section to another space
        uint64 phdr_num = PGROUNDUP((uint64)phdr->p_memsz + phdr->p_vaddr - PGROUNDDOWN(phdr->p_vaddr)) / PGSIZE;
        uint64 *phdr_temp = (uint64 *)alloc_pages(phdr_num);
        memcpy((void *)((uint64)phdr_temp + phdr->p_vaddr - PGROUNDDOWN(phdr->p_vaddr)), 
        (void*)((uint64)&uapp_start + (uint64)phdr->p_offset), (uint64)phdr->p_memsz);
        // printk("%ld\n%ld\n", *phdr_temp, *(uint64*)((uint64)&uapp_start + (uint64)phdr->p_offset));
        // mapping the program section with corresponding size and flag
        create_mapping((uint64*)((uint64)task->pgd+PA2VA_OFFSET), (uint64)PGROUNDDOWN(phdr->p_vaddr), 
        (uint64)phdr_temp-PA2VA_OFFSET, (uint64)phdr_num, (uint64)phdr->p_flags << 1 | 0b10001);
        }
    }

    //用户栈分配
    uint64 user_stack = alloc_page();
    //将用户虚拟地址结尾一页映射到用户的栈，权限为U|-|W|R|V(?)
    create_mapping((uint64*)((uint64)task->pgd+PA2VA_OFFSET), USER_END-PGSIZE, user_stack-PA2VA_OFFSET, 1, 0b10111);
    // pc for the user program
    task->thread.sepc = ehdr->e_entry;
    // other task setting keep same
    task->thread.sstatus = csr_read(sstatus);
    //sstatus的SPP位设置为0，则sret后返回用户模式
    task->thread.sstatus &= ~(1<<8);
    //sstatus的SUM和SPIE位设置为1，则S模式可以访问用户模式内容，且sret后开启中断
    task->thread.sstatus |= 1<<18 | 1<<5;
    //设置sscratch为sp，即为用户虚拟地址结尾
    task->thread.sscratch = USER_END;
}

static uint64_t load_binary(struct task_struct* task) {
    //用户栈分配
    uint64 user_stack = alloc_page();
    //映射uapp和u_mode stack
    uint64 user_pnum = PGROUNDUP(((uint64)&uapp_end - (uint64)&uapp_start))/PGSIZE;
    uint64 *user_temp = (uint64 *)alloc_pages(user_pnum);
    memcpy((void*)user_temp, (void*)&uapp_start, user_pnum*PGSIZE);
    //将用户虚拟地址开始映射到用户的复制好的uapp信息，权限为U|X|W|R|V(?)
    create_mapping((uint64*)((uint64)task->pgd+PA2VA_OFFSET), USER_START, 
    (uint64)user_temp-PA2VA_OFFSET, user_pnum, 0b11111);        
    //将用户虚拟地址结尾一页映射到用户的栈，权限为U|-|W|R|V(?)
    create_mapping((uint64*)((uint64)task->pgd+PA2VA_OFFSET), USER_END-PGSIZE, 
    user_stack-PA2VA_OFFSET, 1, 0b10111);
    // printk("%x\n", ((uint64*)((uint64)task[i]->pgd+PA2VA_OFFSET))[0]);
    //设置sepc为user_start，在虚拟空间上即为0
    task->thread.sepc = USER_START;
    task->thread.sstatus = csr_read(sstatus);
    //sstatus的SPP位设置为0，则sret后返回用户模式
    task->thread.sstatus &= ~(1<<8);
    //sstatus的SUM和SPIE位设置为1，则S模式可以访问用户模式内容，且sret后开启中断
    task->thread.sstatus |= 1<<18 | 1<<5;
    //设置sscratch为sp，即为用户虚拟地址结尾
    task->thread.sscratch = USER_END;
}

void task_init() {
    // 1. 调用 kalloc() 为 idle 分配一个物理页
    // 2. 设置 state 为 TASK_RUNNING;
    // 3. 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
    // 4. 设置 idle 的 pid 为 0
    // 5. 将 current 和 task[0] 指向 idle

    /* YOUR CODE HERE */
    idle = (struct task_struct*)kalloc();  //分配物理页
    idle->state = TASK_RUNNING;
    idle->counter = 0;
    idle->priority = 0;
    idle->pid = 0;
    current = idle;     //当前进程(最开始)为idle
    task[0] = idle;     //task[0]即idle

    // 1. 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
    // 2. 其中每个线程的 state 为 TASK_RUNNING, counter 为 0, priority 使用 rand() 来设置, pid 为该线程在线程数组中的下标。
    // 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`,
    // 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址,  `sp` 设置为 该线程申请的物理页的高地址

    /* YOUR CODE HERE */
    for(int i=1; i < NR_TASKS; i++)
    {
        //为进程分配空间（一个物理页）
        task[i] = (struct task_struct*)kalloc();
        task[i]->counter = 0;
        task[i]->priority = rand()%PRIORITY_MAX + PRIORITY_MIN;  //使用rand赋值优先级
        task[i]->pid = i;
        task[i]->state = TASK_RUNNING;  //所以进程只有running状态
        task[i]->thread.ra = (uint64)&__dummy;  //return address设置为dummy的地址
        task[i]->thread.sp = (uint64)((uint64)task[i]+PGSIZE); //把sp设置为物理页的高低址
        // task[i]->thread_info->kernel_sp = task[i]->thread.sp;   //内核栈针
        // task[i]->thread_info->user_sp = (uint64)(user_stack+PGSIZE);   //用户栈针
        task[i]->pgd = (pagetable_t)(alloc_page()-PA2VA_OFFSET);       //存的是物理地址
        memcpy((void*)((uint64)task[i]->pgd+PA2VA_OFFSET), (void*)(swapper_pg_dir), PGSIZE);           //将内核态页表复制到用户进程

        // load_binary(task[i]);
        load_elf(task[i]);
        
    }
    

    printk("...proc_init done!\n");
}

void dummy() {
    uint64 MOD = 1000000007;
    uint64 auto_inc_local_var = 0;
    int last_counter = -1;
    while(1) {
        if (last_counter == -1 || current->counter != last_counter) {
            last_counter = current->counter;
            auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
            printk("[PID = %d] is running. thread space begin at = 0x%lx\n", current->pid, current);
            // printk("Time Count: %d\n", current->counter); //add by me
        }
    }
}

extern void __switch_to(struct task_struct* prev, struct task_struct* next);

void switch_to(struct task_struct* next) {
    /* YOUR CODE HERE */
    struct task_struct *temp = current;
    //若当前进程pid与下一进程pid相同，不需要切换，继续运行
    if(current->pid == next->pid)
        return;
    //转换current线程
    current = next;     //若没有切换，则会出现异常
    #ifdef SJF
    printk("switch to [PID = %d COUNTER = %d]\n", next->pid, next->counter);
    #endif
    #ifdef PRIORITY
    printk("switch to [PID = %d PRIORITY = %d COUNTER = %d]\n", next->pid, next->priority, next->counter);
    #endif
    __switch_to(temp, next);
}

void do_timer(void) {
    /* YOUR CODE HERE */
    // 1. 如果当前线程是 idle 线程 直接进行调度
    if(current->pid==0)
    {
        schedule();
    }
    // 2. 如果当前线程不是 idle 对当前线程的运行剩余时间减1 若剩余时间仍然大于0 则直接返回 否则进行调度
    else
    {
        current->counter--;
        if(current->counter > 0)
        {
            return; //时间仍然大于0
        }
        else
        {
            schedule(); //进行调度
        }
        
    }
}

#ifdef SJF
void schedule(void) {
    /* YOUR CODE HERE */
    //记录调度进程和最小时间
    struct task_struct *next = task[0];
    uint64 min = 0xffffffffffffffff;
    for(int i=1; i<NR_TASKS; i++)
    {
        //若时间>0，正在运行且最小
        if(task[i]->counter > 0 && task[i]->state == TASK_RUNNING && task[i]->counter < min)
        {
            next = task[i];
            min = task[i]->counter;
        }
    }
    //若全部为0（即next没有动）
    if(next == task[0])
    {
        //随机分配时间
        for(int i=1; i<NR_TASKS; i++)
        {
            task[i]->counter = rand();
            // printk("SET [PID = %d COUNTER = %d]\n", task[i]->pid, task[i]->counter);
        }
        schedule();
        return;
    }
    switch_to(next);
}
#endif

#ifdef PRIORITY
void schedule(void) {
    /* YOUR CODE HERE */
    //记录调度进程和最高优先级
    struct task_struct *next = task[0];
    uint64 max = 0;
    for(int i=1; i<NR_TASKS; i++)
    {
        //若时间>0，正在运行且最大优先级
        if(task[i]->counter > 0 && task[i]->state == TASK_RUNNING && task[i]->priority > max)
        {
            next = task[i];
            max = task[i]->priority;
        }
    }
    //若全部为0（即next没有动）
    if(next == task[0])
    {
        //随机分配时间
        for(int i=1; i<NR_TASKS; i++)
        {
            task[i]->counter = rand();
            // printk("SET [PID = %d PRIORITY = %d COUNTER = %d]\n", task[i]->pid, task[i]->priority, task[i]->counter);
        }
        schedule();
        return;
    }
    switch_to(next);
}
#endif