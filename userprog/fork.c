#include "fork.h"

#include "debug.h"
#include "interrupt.h"
#include "memory.h"
#include "process.h"
#include "string.h"
#include "thread.h"

extern void intr_exit();

/**
 * @brief 把父进程的pcb、虚拟地址位图、0级栈copy给子进程
 * 
 * @param child 
 * @param parent 
 * @return int32_t 
 */
int32_t copy_pcb_vaddrbitmap_stack0(struct pcb* child, struct pcb* parent) {
    memcpy(child, parent, PG_SIZE);
    child->pid = alloc_pid();
    child->elapsed_ticks = 0;
    child->status = TASK_READY;
    child->ticks = child->priority;
    child->parent_pid = parent->pid;
    child->general_node.prev = child->general_node.next = NULL;
    child->all_list_node.prev = child->all_list_node.next = NULL;
    block_desc_init(child->user_block_descs);

    uint32_t cnt = DIV_ROUND_UP((0xc0000000 - USER_VADDR_START) / PG_SIZE / 8, PG_SIZE);
    void* vaddr_btmp = get_kernel_pages(cnt);
    if (vaddr_btmp == NULL) return -1;

    memcpy(vaddr_btmp, child->userprog_vaddr.vaddr_bitmap.bits, cnt * PG_SIZE);
    child->userprog_vaddr.vaddr_bitmap.bits = vaddr_btmp;

    ASSERT(strlen(child->name) < 11);

    uint32_t len = strlen(child->name);
    child->name[len] = '_';
    child->name[len + 1] = 'f';
    child->name[len + 2] = 'o';
    child->name[len + 3] = 'r';
    child->name[len + 4] = 'k';
    child->name[len + 5] = '\0';
    return 0;
}

/**
 * @brief 把父进程的进程体（正在使用的代码段和数据段）、用户栈copy给子进程
 * 
 * @param tran_page 内核空间的中转页
 */
static void copy_body_stack3(struct pcb* child, struct pcb* parent, void* tran_page) {
    uint8_t* vaddr_btmp = parent->userprog_vaddr.vaddr_bitmap.bits;
    uint32_t len = parent->userprog_vaddr.vaddr_bitmap.len;
    uint32_t vaddr_start = parent->userprog_vaddr.vaddr_start;
    uint32_t idx_byte = 0;
    uint32_t idx_bit = 0;
    uint32_t prog_vaddr = 0;

    while (idx_byte < len) {
        if (vaddr_btmp[idx_byte]) {
            idx_bit = 0;
            while (idx_bit < 8) {  // 该页被父进程使用，需要copy
                if ((1 << idx_bit) & vaddr_btmp[idx_byte]) {
                    prog_vaddr = (idx_byte * 8 + idx_bit) * PG_SIZE + vaddr_start;
                    memcpy(tran_page, (void*)prog_vaddr, PG_SIZE);
                    page_dir_activate(child);
                    get_one_page_no_bitmap(PF_USER, prog_vaddr);
                    memcpy((void*)prog_vaddr, tran_page, PG_SIZE);
                    page_dir_activate(parent);
                }
                idx_bit++;
            }
        }
        idx_byte++;
    }
}

/**
 * @brief 创建子进程的线程栈
 * 
 */
int32_t build_child_stack(struct pcb* child) {
    struct intr_stack* intr_stack = (struct intr_stack*)((uint32_t)child + PG_SIZE - sizeof(struct intr_stack));
    intr_stack->eax = 0;  // return 0

    uint32_t* ret_addr = (uint32_t*)intr_stack - 1;
    uint32_t* esi_ptr = (uint32_t*)intr_stack - 2;
    uint32_t* edi_ptr = (uint32_t*)intr_stack - 3;
    uint32_t* ebx_ptr = (uint32_t*)intr_stack - 4;
    uint32_t* ebp_ptr = (uint32_t*)intr_stack - 5;

    *ret_addr = (uint32_t)intr_exit;  // 直接从中断返回
    *ebp_ptr = *ebx_ptr = *edi_ptr = *esi_ptr = 0;
    child->self_kstack = ebp_ptr;
    return 0;
}

/**
 * @brief 将父进程所使用的资源都copy给子进程
 * 
 * @param child 
 * @param parent 
 * @return int32_t 
 */
int32_t copy_process(struct pcb* child, struct pcb* parent) {
    void* tran_page = get_kernel_pages(1);
    if (tran_page == NULL) return -1;

    if (copy_pcb_vaddrbitmap_stack0(child, parent) == -1) return -1;

    child->pg_dir = create_page_dir();
    if (child->pg_dir == NULL) return -1;
    copy_body_stack3(child, parent, tran_page);

    build_child_stack(child);

    mfree_page(PF_KERNEL, tran_page, 1);
    return 0;
}

int16_t sys_fork() {
    struct pcb* parent = running_thread();
    struct pcb* child = get_kernel_pages(1);
    if (child == NULL) return -1;

    ASSERT(INTR_OFF == intr_get_status() && parent->pg_dir != NULL);

    if (copy_process(child, parent) == -1) return -1;

    ASSERT(!node_find(&thread_ready_list, &child->general_node));
    ASSERT(!node_find(&thread_all_list, &child->all_list_node));

    list_append(&thread_ready_list, &child->general_node);
    list_append(&thread_all_list, &child->all_list_node);

    return child->pid;
}
