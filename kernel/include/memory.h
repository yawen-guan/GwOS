// #ifndef __KERNEL_MEMORY_H
// #define __KERNEL_MEMORY_H

#pragma once

#include "bitmap.h"
#include "global.h"
#include "stdint.h"
#include "list.h"

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

// 内存块
struct mem_block {
    struct list_node free_node;
};

// 内存块描述符
struct mem_block_desc {
    uint32_t block_size;        // 内存块的大小
    uint32_t blocks_per_arena;  // 此arena中可包含的内存块数
    struct list free_list;      // 当前可用的内存块列表，可以由多个arena提供
};

#define DESC_CNT 7

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

/**
 * @brief 在pf对应的内存池中申请一页内存，并将虚拟地址vaddr映射到这一页（指定虚拟地址）
 * 
 * @param pf 
 * @param vaddr 
 * @return void* 
 */
void* get_one_page(enum pool_flag pf, uint32_t vaddr);

/**
 * @brief 为vaddr分配一页物理页，但不需要从虚拟地址内存池中设置位图
 * 
 * @return void* 
 */
void* get_one_page_no_bitmap(enum pool_flag pf, uint32_t vaddr);

/**
 * @brief 将一个物理页的对应的内存池的bitmap清零（不改变页表）
 * 
 * @param pg_phy_addr 
 */
void free_one_phy_page(uint32_t pg_phy_addr);

/**
 * @brief 在物理地址池中释放物理页地址、在页表中去掉虚拟地址的映射、在虚拟地址池中释放虚拟地址：释放vaddr开头的连续cnt个物理页
 * 
 */
void mfree_page(enum pool_flag pf, void* void_vaddr, uint32_t cnt);

/**
 * @brief 将虚拟地址转换为物理地址
 * 
 * @return uint32_t 
 */
uint32_t addr_vir2phy(uint32_t vaddr);

/**
 * @brief malloc申请size字节的内存（在堆中）
 * 
 * @param size 
 * @return void* 
 */
void* sys_malloc(uint32_t size);

/**
 * @brief 释放ptr所指向的内存
 * 
 * @param ptr 
 */
void sys_free(void* ptr);

/**
 * @brief 初始化内存块描述符
 * 
 * @param descs 
 */
void block_desc_init(struct mem_block_desc* descs);

// #endif