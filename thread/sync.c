#include "sync.h"

#include "debug.h"
#include "global.h"
#include "interrupt.h"
#include "list.h"
#include "thread.h"

/**
 * @brief 初始化信号量
 * 
 */
void semaphore_init(struct semaphore* sema, uint8_t value) {
    sema->value = value;
    list_init(&sema->waiters);
}

/**
 * @brief 初始化锁
 * 
 */
void lock_init(struct lock* lock) {
    lock->holder = NULL;
    lock->holder_repeat_cnt = 0;
    semaphore_init(&lock->semaphore, 1);
}

/**
 * @brief 信号量 down操作
 * (1) 判断信号量是否大于0
 * (2) 若信号量>0, 信号量-1
 * (3) 若信号量=0, 线程阻塞， 在此信号量上等待
 * 
 * @param sema 
 */
void semaphore_down(struct semaphore* sema) {
    enum intr_status old_status = intr_disable();

    while (sema->value == 0) {  //锁被占用
        list_append(&sema->waiters, &running_thread()->general_node);
        thread_block(TASK_BLOCKED);
    }

    sema->value--;
    ASSERT(sema->value == 0);

    intr_set_status(old_status);
}

/**
 * @brief 信号量 up操作
 * (1) 将信号量的值+1
 * (2) 唤醒在次信号量上等待的线程
 * 
 * @param sema 
 */
void semaphore_up(struct semaphore* sema) {
    enum intr_status old_status = intr_disable();
    if (!list_empty(&sema->waiters)) {
        struct pcb* blocked_thread = elem2entry(struct pcb, general_node, list_pop(&sema->waiters));
        thread_unblock(blocked_thread);
    }
    sema->value++;
    intr_set_status(old_status);
}

/**
 * @brief 获取锁
 * 
 * @param lock 
 */
void lock_acquire(struct lock* lock) {
    if (lock->holder != running_thread()) {
        semaphore_down(&lock->semaphore);
        lock->holder = running_thread();
        lock->holder_repeat_cnt = 1;
    } else  //重复申请
        lock->holder_repeat_cnt++;
}

/**
 * @brief 释放锁
 * 
 * @param lock 
 */
void lock_release(struct lock* lock) {
    if (lock->holder_repeat_cnt > 1) {
        lock->holder_repeat_cnt--;
        return;
    }
    lock->holder = NULL;
    lock->holder_repeat_cnt = 0;
    semaphore_up(&lock->semaphore);
}