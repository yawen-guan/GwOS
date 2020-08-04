#include "wait.h"

#include "bitmap.h"
#include "console.h"
#include "debug.h"
#include "exit.h"
#include "global.h"
#include "list.h"
#include "memory.h"
#include "print.h"
#include "stdio.h"
#include "syscall.h"
#include "thread.h"

/**
 * @brief 释放进程所占用的资源：页表映射、内存池中的物理页
 * 
 * @param thread 
 */
void release_prog_resource(struct pcb* thread) {
    uint32_t* pgdir_vaddr = thread->pg_dir;
    uint16_t user_pde_nr = 768, pde_idx = 0;
    uint32_t pde = 0;
    uint32_t* ptr_pde = NULL;

    uint16_t user_pte_nr = 1024, pte_idx = 0;
    uint32_t pte = 0;
    uint32_t* ptr_pte = NULL;

    uint32_t* first_pte_vaddr_in_pde = NULL;
    uint32_t pg_phy_addr = 0;

    // 页表
    while (pde_idx < user_pde_nr) {
        ptr_pde = pgdir_vaddr + pde_idx;
        pde = *ptr_pde;
        if (pde & 0x00000001) {  // 有页表项
            first_pte_vaddr_in_pde = pte_ptr(pde_idx * 0x400000);
            pte_idx = 0;
            while (pte_idx < user_pte_nr) {
                ptr_pte = first_pte_vaddr_in_pde + pte_idx;
                pte = *ptr_pte;
                if (pte & 0x00000001) {
                    pg_phy_addr = pte & 0xfffff000;
                    free_one_phy_page(pg_phy_addr);
                }
                pte_idx++;
            }
            pg_phy_addr = pde & 0xfffff000;
            free_one_phy_page(pg_phy_addr);
        }
        pde_idx++;
    }

    // 物理页
    uint32_t cnt = (thread->userprog_vaddr.vaddr_bitmap.len) / PG_SIZE;
    uint8_t* user_vaddr_pool_bitmap = thread->userprog_vaddr.vaddr_bitmap.bits;
    mfree_page(PF_KERNEL, user_vaddr_pool_bitmap, cnt);
}

bool check_child(struct list_node* node, int32_t ppid) {
    struct pcb* thread = elem2entry(struct pcb, all_list_node, node);
    if (thread->parent_pid == ppid) return true;
    return false;
}

bool check_hanging_child(struct list_node* node, int32_t ppid) {
    struct pcb* thread = elem2entry(struct pcb, all_list_node, node);
    if (thread->parent_pid == ppid && thread->status == TASK_HANGING) return true;
    return false;
}

bool change_parent_to_init(struct list_node* node, int32_t ppid) {
    struct pcb* thread = elem2entry(struct pcb, all_list_node, node);
    if (thread->parent_pid == ppid) {
        thread->parent_pid = 1;  // init
    }
    return false;
}

int16_t sys_wait_without_pid(int32_t* child_exit_status) {
    struct pcb* parent = running_thread();
    while (1) {
        // hanging
        struct list_node* node = list_traversal(&thread_all_list, check_hanging_child, parent->pid);
        if (node != NULL) {
            struct pcb* child = elem2entry(struct pcb, all_list_node, node);
            // printf("child: %s\n", child->name);
            *child_exit_status = child->exit_status;
            uint16_t child_pid = child->pid;
            thread_exit(child, false);
            return child_pid;
        }
        // not hanging: should wait for child to finish
        node = list_traversal(&thread_all_list, check_child, parent->pid);
        if (node == NULL) return -1;
        thread_block(TASK_WAITING);
    }
}

bool check_thread(struct list_node* node, int32_t pid) {
    struct pcb* thread = elem2entry(struct pcb, all_list_node, node);
    if (thread->pid == pid) return true;
    return false;
}

bool check_hanging_thread(struct list_node* node, int32_t pid) {
    struct pcb* thread = elem2entry(struct pcb, all_list_node, node);
    if (thread->pid == pid && thread->status == TASK_HANGING) return true;
    return false;
}

int16_t sys_wait(int32_t pid, int32_t* child_exit_status) {
    struct pcb* parent = running_thread();
    while (1) {
        // hanging
        struct list_node* node = list_traversal(&thread_all_list, check_hanging_thread, pid);
        if (node != NULL) {
            struct pcb* child = elem2entry(struct pcb, all_list_node, node);
            *child_exit_status = child->exit_status;
            uint16_t child_pid = child->pid;
            thread_exit(child, false);
            return child_pid;
        }
        // not hanging: should wait for child to finish
        node = list_traversal(&thread_all_list, check_thread, pid);
        if (node == NULL) return -1;
        thread_block(TASK_WAITING);
    }
}