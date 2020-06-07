#include "thread.h"

#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "string.h"

#define PG_SIZE 4096  // 4KB/page
#define STACK_MAGIC 0x12345678

struct pcb *main_thread;        // 主线程PCB
struct list thread_ready_list;  // 就绪队列
struct list thread_all_list;    // 所有任务队列
static struct list_node *thread_node;

extern void switch_to(struct pcb *now, struct pcb *next);

/**
 * @brief 获取当前运行中线程的pcb指针
 * 各个线程所用的0级栈都在自己的PCB中，所以当前栈指针的高20位是当前运行线程的pcb的起始地址
 * @return struct pcb* 
 */
struct pcb *running_thread() {
    uint32_t esp;
    asm("mov %%esp, %0"
        : "=g"(esp));
    return (struct pcb *)(esp & 0xfffff000);
}

/**
 * @brief 执行函数
 * 
 * @param func 函数指针
 * @param func_arg 函数参数 
 */
void kernel_thread(thread_func *func, void *func_arg) {
    intr_enable();
    func(func_arg);
}

/**
 * @brief 初始化线程栈
 * 
 */
void thread_create(struct pcb *pthread, thread_func func, void *func_arg) {
    //保留中断栈
    pthread->self_kstack -= sizeof(struct intr_stack);
    //保留线程栈
    pthread->self_kstack -= sizeof(struct thread_stack);

    //初始化内核线程栈
    struct thread_stack *ks = (struct thread_stack *)pthread->self_kstack;
    ks->eip = kernel_thread;
    ks->func = func;
    ks->func_arg = func_arg;
    ks->ebp = 0;
    ks->ebx = 0;
    ks->esi = 0;
    ks->edi = 0;
}

/**
 * @brief 初始化线程
 * 
 */
void init_thread(struct pcb *pthread, char *name, int priority) {
    memset(pthread, 0, sizeof(*pthread));
    pthread->self_kstack = (uint32_t *)((uint32_t)pthread + PG_SIZE);  // 0特权级栈，初始化为pcb最顶端
    if (pthread == main_thread) {
        pthread->status = TASK_RUNNING;
    } else {
        pthread->status = TASK_READY;
    }
    strcpy(pthread->name, name);
    pthread->priority = priority;
    pthread->ticks = priority;
    pthread->elapsed_ticks = 0;
    pthread->pg_dir = NULL;
    pthread->stack_magic = STACK_MAGIC;
}

/**
 * @brief 创建线程
 * 
 * @param name 线程名
 * @param prioriry 优先级 
 * @param func 线程执行的函数
 * @param func_arg 线程执行的函数的参数
 * @return struct pcb* 
 */
struct pcb *thread_start(char *name, int priority, thread_func func, void *func_arg) {
    put_str("thread_start\n", 0x07);

    struct pcb *thread = get_kernel_pages(1);  //pcb占一页

    put_str("finish get_kernel_pages\n", 0x07);

    init_thread(thread, name, priority);
    thread_create(thread, func, func_arg);

    list_append(&thread_ready_list, &thread->general_node);
    list_append(&thread_all_list, &thread->all_list_node);

    // asm volatile("movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret"
    //              :
    //              : "g"(thread->self_kstack)
    //              : "memory");

    put_str("thread_end\n", 0x07);
    return thread;
}

/**
 * @brief 为主线程创建pcb
 * 
 */
void make_main_thread() {
    main_thread = running_thread();
    init_thread(main_thread, "main", 31);

    list_append(&thread_all_list, &main_thread->all_list_node);
}

/**
 * @brief 调度
 * 
 */
void schedule() {
    struct pcb *now = running_thread();
    if (now->status == TASK_RUNNING) {  //time slice用完
        list_append(&thread_ready_list, &now->general_node);
        now->ticks = now->priority;
        now->status = TASK_READY;
    } else {
    }

    if (list_empty(&thread_ready_list)) {
        set_cursor_in_pos(0, 0);
        put_str("thread_ready_list is empty!\n", 0x07);
        while (1)
            ;
    }
    thread_node = NULL;  // thread_tag清空
                         /* 将thread_ready_list队列中的第一个就绪线程弹出,准备将其调度上cpu. */
    thread_node = list_pop(&thread_ready_list);
    struct pcb *next = elem2entry(struct pcb, general_node, thread_node);
    next->status = TASK_RUNNING;
    switch_to(now, next);
}

/**
 * @brief 线程环境初始化
 * 
 */
void thread_init() {
    put_str("thread_init start\n", 0x07);

    list_init(&thread_ready_list);
    list_init(&thread_all_list);

    make_main_thread();

    put_str("thread_init done\n", 0x07);
}