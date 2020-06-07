#include "thread.h"

#include "memory.h"
#include "string.h"

#define PG_SIZE 4096  // 4KB/page
#define STACK_MAGIC 0x12345678

/**
 * @brief 执行函数
 * 
 * @param func 函数指针
 * @param func_arg 函数参数 
 */
void kernel_thread(thread_func *func, void *func_arg) {
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
    pthread->status = TASK_RUNNING;
    pthread->priority = priority;
    strcpy(pthread->name, name);
    pthread->self_kstack = (uint32_t *)((uint32_t)pthread + PG_SIZE);  // 0特权级栈，初始化为pcb最顶端
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

    asm volatile("movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret"
                 :
                 : "g"(thread->self_kstack)
                 : "memory");

    put_str("thread_end\n", 0x07);
    return thread;
}