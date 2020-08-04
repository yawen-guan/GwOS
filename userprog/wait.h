#pragma once

#include "thread.h"
#include "list.h"
#include "global.h"
#include "stdint.h"

/**
 * @brief 释放进程所占用的资源：页表映射、内存池中的物理页
 * 
 * @param thread 
 */
void release_prog_resource(struct pcb* thread);

bool check_child(struct list_node* node, int32_t ppid);

bool check_hanging_child(struct list_node* node, int32_t ppid);

bool change_parent_to_init(struct list_node* node, int32_t ppid);

/**
 * @brief 父进程等待子进程
 * 
 */
int16_t sys_wait(int32_t pid, int32_t* child_exit_status);

int16_t sys_wait_without_pid(int32_t* child_exit_status);