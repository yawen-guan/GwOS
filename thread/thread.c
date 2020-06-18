#include "thread.h"

#include "console.h"
#include "debug.h"
#include "global.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "string.h"
#include "sync.h"
#include "syscall.h"

#define PG_SIZE 4096  // 4KB/page
#define STACK_MAGIC 0x12345678

struct pcb *main_thread;        // 主线程PCB
struct list thread_ready_list;  // 就绪队列
struct list thread_all_list;    // 所有任务队列
static struct list_node *thread_node;
struct lock pid_lock;

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

int16_t alloc_pid() {
    static int16_t next_pid = 0;
    lock_acquire(&pid_lock);
    next_pid++;
    lock_release(&pid_lock);
    return next_pid;
}

/**
 * @brief 初始化线程栈
 * 
 */
void thread_create(struct pcb *thread, thread_func func, void *func_arg) {
    //保留中断栈
    thread->self_kstack -= sizeof(struct intr_stack);
    //保留线程栈
    thread->self_kstack -= sizeof(struct thread_stack);

    //初始化内核线程栈
    struct thread_stack *ks = (struct thread_stack *)thread->self_kstack;
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
void init_thread(struct pcb *thread, char *name, int priority) {
    memset(thread, 0, sizeof(*thread));
    thread->self_kstack = (uint32_t *)((uint32_t)thread + PG_SIZE);  // 0特权级栈，初始化为pcb最顶端
    thread->pid = alloc_pid();
    if (thread == main_thread) {
        thread->status = TASK_RUNNING;
    } else {
        thread->status = TASK_READY;
    }
    strcpy(thread->name, name);
    thread->priority = priority;
    thread->ticks = priority;
    thread->elapsed_ticks = 0;
    thread->pg_dir = NULL;
    thread->stack_magic = STACK_MAGIC;
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
    struct pcb *thread = get_kernel_pages(1);  //pcb占一页

    init_thread(thread, name, priority);
    thread_create(thread, func, func_arg);

    ASSERT(!node_find(&thread_ready_list, &thread->general_node));
    ASSERT(!node_find(&thread_all_list, &thread->all_list_node));

    list_append(&thread_ready_list, &thread->general_node);
    list_append(&thread_all_list, &thread->all_list_node);

    return thread;
}

/**
 * @brief 为主线程创建pcb
 * 
 */
void make_main_thread() {
    main_thread = running_thread();
    init_thread(main_thread, "main", 31);

    ASSERT(!node_find(&thread_all_list, &main_thread->all_list_node));
    list_append(&thread_all_list, &main_thread->all_list_node);
}

/**
 * @brief 调度线程：将当前线程换下， 换上下一个线程
 * 
 */
void schedule() {
    ASSERT(intr_get_status() == INTR_OFF);

    struct pcb *now = running_thread();

    if (now->status == TASK_RUNNING) {  //time slice用完
        // debug_printf_s("task-running ", "ok");
        ASSERT(!node_find(&thread_ready_list, &now->general_node));
        list_append(&thread_ready_list, &now->general_node);
        now->ticks = now->priority;
        now->status = TASK_READY;
    } else {
        //等待某事件发生
    }

    // debug_printf_uint("is empty ", list_empty(&thread_ready_list), 10);
    // struct list_node *node = &thread_ready_list.head;
    // while (node != &thread_ready_list.tail) {
    //     console_debug_printf_str("node ", "here");
    //     node = node->next;
    // }

    ASSERT(list_empty(&thread_ready_list) == false);
    thread_node = NULL;
    thread_node = list_pop(&thread_ready_list);

    struct pcb *next = elem2entry(struct pcb, general_node, thread_node);
    next->status = TASK_RUNNING;
    process_activate(next);

    // debug_printf_s(now->name, next->name);

    switch_to(now, next);
}

/**
 * @brief 线程环境初始化
 * 
 */
void thread_init() {
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    lock_init(&pid_lock);
    make_main_thread();
}

/**
 * @brief 将当前运行的线程阻塞，状态改变为status
 * 
 * @param status 应为TASK_BLOCKED, TASK_WAITING, TASK_HANDING之一
 * 
 */
void thread_block(enum task_status status) {
    ASSERT(((status == TASK_BLOCKED) || (status == TASK_WAITING) || (status == TASK_HANGING)));
    enum intr_status old_status = intr_disable();
    struct pcb *now_thread = running_thread();
    now_thread->status = status;
    schedule();
    intr_set_status(old_status);
}

/**
 * @brief 将线程由阻塞态回复到ready态
 * 
 * @param thread 
 */
void thread_unblock(struct pcb *thread) {
    ASSERT(((thread->status == TASK_BLOCKED) || (thread->status == TASK_WAITING) || (thread->status == TASK_HANGING)));

    enum intr_status old_status = intr_disable();
    if (thread->status != TASK_READY) {
        ASSERT(!node_find(&thread_ready_list, &thread->general_node));

        list_push(&thread_ready_list, &thread->general_node);
        thread->status = TASK_READY;
    }
    intr_set_status(old_status);
}