//arch/riscv/kernel/proc.c
#include "proc.h"
extern void __dummy();
extern int printk(const char *, ...);
struct task_struct* idle;           // idle process
struct task_struct* current;        // 指向当前运行线程的 `task_struct`
struct task_struct* task[NR_TASKS]; // 线程数组, 所有的线程都保存在此

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
            printk("[PID = %d] is running. auto_inc_local_var = %d\n", current->pid, auto_inc_local_var);
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
    #elif PRIORITY
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
            printk("SET [PID = %d COUNTER = %d]\n", task[i]->pid, task[i]->counter);
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
            printk("SET [PID = %d PRIORITY = %d COUNTER = %d]\n", task[i]->pid, task[i]->priority, task[i]->counter);
        }
        schedule();
        return;
    }
    switch_to(next);
}
#endif