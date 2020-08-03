#pragma once

#include "stdint.h"
#include "thread.h"

#define USER_STACK3_VADDR (0xc0000000 - 0x1000)
#define USER_VADDR_START 0x8048000

#define default_priority 31

/**
 * @brief 创建用户进程，将其加入到ready list等待执行
 * 
 * @param process_name 
 * @param name 
 */
void process_execute(void* process_name, char* name);

/**
 * @brief 激活进程或线程对应的页表并更新进程的特权级0栈（tss的esp0）
 * 
 * @param thread 
 */
void process_activate(struct pcb* thread);

/**
 * @brief 激活页表, 把用户进程的页表加载到CR3寄存器
 * 用户进程、内核线程都需要页表的切换
 * 
 * @param thread 
 */
void page_dir_activate(struct pcb* thread);

/**
 * @brief 创建页目录表
 * 共享内核：把内核页目录表的768-1026个项复制到用户进程页表的对应位置
 * 
 * @return uint32_t*  页目录的虚拟地址，若失败返回-1
 */
uint32_t* create_page_dir();