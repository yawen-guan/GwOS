#include "process.h"

#include "console.h"
#include "debug.h"
#include "global.h"
#include "interrupt.h"
#include "list.h"
#include "memory.h"
#include "print.h"
#include "string.h"
#include "syscall.h"
#include "thread.h"
#include "tss.h"

extern void intr_exit(void);  // kernel.asm中定义，从中断返回

/**
 * @brief 构造用户进程的上下文
 * 创建用户进程的上下文，也就是填充用户进程的intr_stack，通过假装从中断返回，间接运行用户进程
 * 
 * @param proc
 */
void start_process(void* proc) {
    void* function = proc;
    struct pcb* now = running_thread();
    now->self_kstack += sizeof(struct thread_stack);  //指向intr_stack最低处
    struct intr_stack* proc_stack = (struct intr_stack*)now->self_kstack;
    proc_stack->edi = proc_stack->esi = proc_stack->ebp = proc_stack->esp_dummy = 0;
    proc_stack->ebx = proc_stack->edx = proc_stack->ecx = proc_stack->eax = 0;
    proc_stack->gs = 0;  //用户态不需要gs
    proc_stack->ds = proc_stack->es = proc_stack->fs = SELECTOR_USER_DATA;
    proc_stack->eip = function;
    proc_stack->cs = SELECTOR_USER_CODE;
    proc_stack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    proc_stack->esp = (void*)((uint32_t)get_one_page(PF_USER, USER_STACK3_VADDR) + PG_SIZE);  ///////  here
    proc_stack->ss = SELECTOR_USER_DATA;
    asm volatile("movl %0, %%esp; jmp intr_exit"
                 :
                 : "g"(proc_stack)
                 : "memory");
    //把esp替换为proc_stack, 利用intr_exit将proc_stack的数据载入寄存器，退出伪装的中断后进入特权级3
}

/**
 * @brief 激活页表, 把用户进程的页表加载到CR3寄存器
 * 用户进程、内核线程都需要页表的切换
 * 
 * @param thread 
 */
void page_dir_activate(struct pcb* thread) {
    uint32_t pagedir_phy_addr = 0x100000;  //所有内核线程的页表地址
    if (thread->pg_dir != NULL) {          //用户进程
        pagedir_phy_addr = addr_vir2phy((uint32_t)thread->pg_dir);
    }

    //刷新页表
    asm volatile("movl %0, %%cr3"
                 :
                 : "r"(pagedir_phy_addr)
                 : "memory");
}

/**
 * @brief 激活进程或线程对应的页表，更新进程的特权级0栈（tss的esp0）
 * 进程或线程被中断信号打断后会进入0特权级，在0级栈中保存上下文环境
 * 
 * @param thread 
 */
void process_activate(struct pcb* thread) {
    ASSERT(thread != NULL);
    page_dir_activate(thread);
    if (thread->pg_dir) {  //用户进程才需要更新tss的esp0
        update_tss_esp(thread);
    }
    ASSERT(thread->status == TASK_RUNNING);
}

/**
 * @brief 创建页目录表
 * 共享内核：把内核页目录表的768-1026个项复制到用户进程页表的对应位置
 * 
 * @return uint32_t*  页目录的虚拟地址，若失败返回-1
 */
uint32_t* create_page_dir() {
    uint32_t* page_dir_vaddr = get_kernel_pages(1);  //用户页表在内核空间
    if (page_dir_vaddr == NULL) {
        console_put_str("create_page_dir: get_kernel_page failed!", 0x07);
        return NULL;
    }

    //复制页表 768-1023
    memcpy((uint32_t*)((uint32_t)page_dir_vaddr + 0x300 * 4), (uint32_t*)(0xfffff000 + 0x300 * 4), 1024);

    //把用户页目录表的最后一项更新为该页目录表的物理地址
    uint32_t new_page_dir_phy_addr = addr_vir2phy((uint32_t)page_dir_vaddr);
    page_dir_vaddr[1023] = new_page_dir_phy_addr | PG_US_U | PG_RW_W | PG_P_1;
    return page_dir_vaddr;
}

/**
 * @brief 初始化用户进程的虚拟地址的bitmap
 * 
 * @param user_prog 
 */
void create_user_vaddr_bitmap(struct pcb* user_prog) {
    user_prog->userprog_vaddr.vaddr_start = USER_VADDR_START;
    uint32_t bitmap_pg_cnt = DIV_ROUND_UP((0xc0000000 - USER_VADDR_START) / PG_SIZE / 8, PG_SIZE);
    user_prog->userprog_vaddr.vaddr_bitmap.bits = get_kernel_pages(bitmap_pg_cnt);
    user_prog->userprog_vaddr.vaddr_bitmap.len = (0xc0000000 - USER_VADDR_START) / PG_SIZE / 8;
    bitmap_init(&user_prog->userprog_vaddr.vaddr_bitmap);
}

/**
 * @brief 创建用户进程，将其加入到ready list等待执行
 * 
 * @param process_name 
 * @param name 
 */
void process_execute(void* proc, char* name) {
    struct pcb* thread = get_kernel_pages(1);  //pcb在内核中

    ASSERT(thread != NULL);

    init_thread(thread, name, default_priority);
    create_user_vaddr_bitmap(thread);
    thread_create(thread, start_process, proc);
    thread->pg_dir = create_page_dir();
    block_desc_init(thread->user_block_descs);

    enum intr_status old_status = intr_disable();
    ASSERT(!node_find(&thread_ready_list, &thread->general_node));
    ASSERT(!node_find(&thread_all_list, &thread->all_list_node));

    list_append(&thread_ready_list, &thread->general_node);
    list_append(&thread_all_list, &thread->all_list_node);

    intr_set_status(old_status);
}
