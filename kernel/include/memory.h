// #ifndef __KERNEL_MEMORY_H
// #define __KERNEL_MEMORY_H

#pragma once

#include "bitmap.h"
#include "global.h"
#include "stdint.h"

// 内存池标记，表示用哪个内存池
enum pool_flag {
    PF_KERNEL = 1,
    PF_USER = 2
};

// page table
#define PG_P_1 1   // P=1,该表存在物理内存中
#define PG_P_0 0   // P=0,该表不在物理内存中
#define PG_RW_R 0  // RW=0, 可读不可写
#define PG_RW_W 2  // RW=1, 可读可写
#define PG_US_S 0  // US=0, Supervisor, 特权级012才可访问该页
#define PG_US_U 4  // US=1, User, 特权级0123均可访问该页

struct virtual_addr {
    struct bitmap vaddr_bitmap;
    uint32_t vaddr_start;  // 虚拟地址的起始地址
};

extern struct pool kernel_pool, user_pool;

/**
 * @brief 求虚拟地址vaddr对应的pte指针
 * 
 * @return pte指针
 */
uint32_t* pte_ptr(uint32_t vaddr);

/**
 * @brief 求虚拟地址vaddr对应的pde指针
 * 
 * @return pde指针
 */
uint32_t* pde_ptr(uint32_t vaddr);

/**
 * @brief 在pf对应的内存池中分配cnt个页
 * 
 * @return 成功：返回起始虚拟地址；失败：返回NULL 
 */
void* malloc_page(enum pool_flag pf, uint32_t cnt);

/**
 * @brief 从内核内存池中申请cnt页
 * 
 * @return 成功：返回虚拟地址；失败：返回NULL
 */
void* get_kernel_pages(uint32_t cnt);

/**
 * @brief 初始化内存
 * 
 */
void mem_init();

// #endif