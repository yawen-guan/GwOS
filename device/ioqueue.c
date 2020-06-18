#include "ioqueue.h"

#include "debug.h"
#include "global.h"
#include "interrupt.h"
#include "print.h"

/**
 * @brief 初始化io环状队列
 * 
 * @param q 
 */
void ioqueue_init(struct ioqueue* q) {
    lock_init(&q->lock);
    q->producer = q->consumer = NULL;
    q->head = q->tail = 0;
}

/**
 * @brief 获得下一个位置的下标
 * 
 * @param pos 
 * @return int32_t 
 */
int32_t get_next_pos(int32_t pos) {
    return (pos + 1) % BUF_SIZE;
}

/**
 * @brief 判断队列是否已满
 * 
 * @param q 
 * @return true 
 * @return false 
 */
bool is_ioq_full(struct ioqueue* q) {
    ASSERT(intr_get_status() == INTR_OFF);
    return get_next_pos(q->head) == q->tail;  //注意是head的下一个与tail碰撞！ioq头进（写）尾出（读）
}

/**
 * @brief 判断队列是否为空
 * 
 * @param q 
 * @return true 
 * @return false 
 */
bool is_ioq_empty(struct ioqueue* q) {
    ASSERT(intr_get_status() == INTR_OFF);
    return q->head == q->tail;
}

/**
 * @brief 让producer or consumer阻塞。在当前缓冲区内等待
 * 
 * @param waiter 指针，指向一个线程指针(pcb *). waiter = ioq->producer or ioq->consumer
 */
void ioq_wait(struct pcb** waiter) {
    ASSERT(*waiter == NULL && waiter != NULL);
    *waiter = running_thread();
    thread_block(TASK_BLOCKED);
}

/**
 * @brief 唤醒waiter（producer or consumer）并将waiter只看置空
 * 
 * @param waiter 
 */
void wakeup(struct pcb** waiter) {
    ASSERT(*waiter != NULL);
    thread_unblock(*waiter);
    *waiter = NULL;
}

/**
 * @brief 从ioq队列中获取一个字符
 * 
 * @param q 
 * @return char 
 */
char ioq_getchar(struct ioqueue* q) {
    ASSERT(intr_get_status() == INTR_OFF);

    while (is_ioq_empty(q) == true) {
        lock_acquire(&q->lock);
        ioq_wait(&q->consumer);
        lock_release(&q->lock);
    }

    char c = q->buf[q->tail];
    q->tail = get_next_pos(q->tail);

    if (q->producer != NULL) {  // 说明生产者因为缓冲区满而阻塞了
        wakeup(&q->producer);
    }
    return c;
}

/**
 * @brief 向ioq队列中写入一个字符
 * 
 * @param q ioq队列
 * @param c 要写入的字符
 */
void ioq_putchar(struct ioqueue* q, char c) {
    ASSERT(intr_get_status() == INTR_OFF);

    while (is_ioq_full(q) == true) {
        lock_acquire(&q->lock);
        ioq_wait(&q->producer);
        lock_acquire(&q->lock);
    }
    q->buf[q->head] = c;
    q->head = get_next_pos(q->head);

    if (q->consumer != NULL) {  //说明消费者因为缓冲区空而阻塞了
        wakeup(&q->consumer);
    }
}
