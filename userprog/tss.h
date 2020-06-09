#pragma once

#include "thread.h"

/**
 * @brief 初始化tss并将其安装到gdt中，另外在GDT中安装DPL=3的数据段和代码段描述符，供用户进程使用
 * 
 */
void tss_init();