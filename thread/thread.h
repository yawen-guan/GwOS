#ifndef __THREAD_H
#define __THREAD_H

#include "stdint.h"

enum task_status {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANDING,
    TASK_DIED
};

typedef void thread_func(void *);  // 通用函数类型

//中断栈：用于中断时保护进程或线程的上下文环境，在线程自己的内核栈的最顶端（位置固定）
struct intr_stack {
    uint32_t vector_id;  // 中断号
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;  // 虽然pushad把esp也压入,但esp是不断变化的,所以会被popad忽略
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    /* 以下由cpu从低特权级进入高特权级时压入 */
    uint32_t err_code;  // err_code会被压入在eip之后
    void (*eip)(void);
    uint32_t cs;
    uint32_t eflags;
    void *esp;
    uint32_t ss;
};

//线程栈：用于存储线程中待执行的函数，在线程自己的内核栈中(位置不固定)
struct thread_stack {
    uint32_t ebp, ebx, edi, esi;

    void (*eip)(thread_func *func, void *func_arg);  // function pointer
    void(*unused_retaddr);                           // 占位
    thread_func *func;
    void *func_arg;
};

struct pcb {
    uint32_t *self_kstack;  // 自己的内核栈
    enum task_status status;
    uint8_t priority;
    char name[16];
    uint32_t stack_magic;  //魔数， 栈的边界标记
};

/**
 * @brief 初始化线程栈
 * 
 */
void thread_create(struct pcb *pthread, thread_func func, void *func_arg);

/**
 * @brief 初始化线程
 * 
 */
void init_thread(struct pcb *pthread, char *name, int priority);
/**
 * @brief 创建线程
 * 
 * @param name 线程名
 * @param prioriry 优先级 
 * @param func 线程执行的函数
 * @param func_arg 线程执行的函数的参数
 * @return struct pcb* 
 */
struct pcb *thread_start(char *name, int prioriry, thread_func func, void *func_arg);

#endif