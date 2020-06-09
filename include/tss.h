#pragma once

#include "thread.h"

/**
 * @brief 初始化tss并将其安装到gdt中，另外在GDT中安装DPL=3的数据段和代码段描述符，供用户进程使用
 * 
 */
void tss_init();

/**
 * @brief 更新TSS中的esp0
 * 只修改0特权级对应的栈, 也就是thread的PCB所在页的最顶端
 * 
 * @param thread 
 */
void update_tss_esp(struct pcb* thread);