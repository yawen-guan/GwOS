#include "process.h"

#include "console.h"
#include "debug.h"
#include "global.h"
#include "interrupt.h"
#include "list.h"
#include "memory.h"
#include "string.h"
#include "thread.h"
#include "tss.h"

extern void intr_exit(void);

/**
 * @brief 构造用户进程的上下文
 * 
 * @param process_name 
 */
void start_process(void* process_name) {
    void* function = process_name;
    struct pcb* now = running_thread();
    now->self_kstack += sizeof(struct thread_stack);
    struct intr_stack* proc_stack = (struct intr_stack*)now->self_kstack;
    proc_stack->edi = proc_stack->esi = proc_stack->ebp = proc_stack->esp_dummy = 0;
    proc_stack->ebx = proc_stack->edx = proc_stack->ecx = proc_stack->eax = 0;
    proc_stack->gs = 0;
    proc_stack->ds = proc_stack->es = proc_stack->fs = SELECTOR_USER_DATA;
    proc_stack->eip = function;
    proc_stack->cs = SELECTOR_USER_CODE;
    proc_stack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    proc_stack->esp = (void*)((uint32_t)get_one_page(PF_USER, USER_STACK3_VADDR) + PG_SIZE);
    proc_stack->ss = SELECTOR_USER_DATA;
    asm volatile("movl %0, %%esp; jmp intr_exit"
                 :
                 : "g"(proc_stack)
                 : "memory");
}

/**
 * @brief 激活页表
 * 用户进程、内核线程都需要页表的切换
 * 
 * @param thread 
 */
void page_dir_activate(struct pcb* thread) {
    uint32_t pagedir_phy_addr = 0x100000;  //内核线程
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
 * @brief 激活进程或线程对应的页表并更新进程的特权级0栈（tss的esp0）
 * 
 * @param thread 
 */
void process_activate(struct pcb* thread) {
    ASSERT(thread != NULL);

    page_dir_activate(thread);

    if (thread->pg_dir != NULL) {  //用户进程才需要更新tss的esp0
        update_tss_esp(thread);
    }
}

/**
 * @brief 创建页目录表
 * 
 * @return uint32_t*  页目录的虚拟地址，若失败返回-1
 */
uint32_t* create_page_dir() {
    /* 用户进程的页表不能让用户直接访问到,所以在内核空间来申请 */
    uint32_t* page_dir_vaddr = get_kernel_pages(1);
    if (page_dir_vaddr == NULL) {
        console_put_str("create_page_dir: get_kernel_page failed!", 0x07);
        return NULL;
    }

    //复制页表
    memcpy((uint32_t*)((uint32_t)page_dir_vaddr + 0x300 * 4), (uint32_t*)(0xfffff000 + 0x300 * 4), 1024);

    uint32_t new_page_dir_phy_addr = addr_vir2phy((uint32_t)page_dir_vaddr);
    page_dir_vaddr[1023] = new_page_dir_phy_addr | PG_US_U | PG_RW_W | PG_P_1;
    return page_dir_vaddr;
}

/**
 * @brief 用户进程的虚拟地址的bitmap的创建
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
void process_execute(void* process_name, char* name) {
    struct pcb* thread = get_kernel_pages(1);  //pcb在内核中
    init_thread(thread, name, default_priority);
    create_user_vaddr_bitmap(thread);
    thread_create(thread, start_process, process_name);
    thread->pg_dir = create_page_dir();

    enum intr_status old_status = intr_disable();
    ASSERT(!node_find(&thread_ready_list, &thread->general_node));
    list_append(&thread_ready_list, &thread->general_node);

    ASSERT(!node_find(&thread_all_list, &thread->all_list_node));
    list_append(&thread_all_list, &thread->all_list_node);
    intr_set_status(old_status);
}
