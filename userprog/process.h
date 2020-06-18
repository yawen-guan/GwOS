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