#pragma once

#include "list.h"
#include "stdint.h"
#include "thread.h"

//信号量
struct semaphore {
    uint8_t value;
    struct list waiters;  // 在此信号量上阻塞的所有线程
};

struct lock {
    struct pcb* holder;
    struct semaphore semaphore;
    // 锁的holder重复申请锁的次数, 避免线程在未释放锁之前，重复调用申请锁的函数的情况下，内外层函数对同一个锁释放两次
    uint32_t holder_repeat_cnt;
};

/**
 * @brief 初始化锁
 * 
 */
void lock_init(struct lock* lock);

/**
 * @brief 获取锁
 * 
 * @param lock 
 */
void lock_acquire(struct lock* lock);

/**
 * @brief 释放锁
 * 
 * @param lock 
 */
void lock_release(struct lock* lock);